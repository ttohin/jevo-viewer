
#pragma once

#include <fstream>
#include <sstream>
#include "json_safe.hpp"
#include "UICommon.h"
#include "Buffer2D.h"
#include "AsyncDiffReader.h"
#include <algorithm>
#include "ObjectContext.h"

namespace jevo
{
  namespace graphic
  {
    class ObjectContext;
  }
  
  using ObjectContextPtr = std::shared_ptr<graphic::ObjectContext>;
  
  class KeyFrameItem
  {
  public:
    cocos2d::Color3B color;
    ObjectContextPtr context;
    uint64_t id;
    Vec2 pos;
  };
  
  enum class DiffType
  {
    Add,
    Move,
    Delete
  };
  
  class WorldModelDiff
  {
  public:
    DiffType type;
    ObjectContextPtr context;
    KeyFrameItem* destinationItem;
    Vec2 sourcePos;
    Vec2 destinationPos;
    uint64_t id;
    
    std::string Description() const
    {
      std::stringstream ss;
      ss <<
      "[" <<
      "WorldModelDiff: " << static_cast<const void*>(this) <<
      " id: " << id <<
      " context: " << (context ? context->Description() : "null") <<
      " destinationItem: " << static_cast<const void*>(destinationItem) <<
      " src: " << sourcePos.Description() <<
      " des: " << destinationPos.Description() <<
      "]";
      return ss.str();
    }
  };
  
  using WorldModelDiffVect = std::vector<WorldModelDiff>;
  
  class KeyFrame
  {
  public:
    
    using BufferType = Buffer2D<KeyFrameItem>;
    using BufferTypePtr = std::shared_ptr<BufferType>;
    
    bool ReadFromFile(const std::string& fileName)
    {
      std::ifstream i(fileName);
      if (!i)
        return false;
      
      nlohmann::json json;
      i >> json;
      
      if (json.is_null())
        return false;
      
      m_width = json["width"];
      m_height = json["height"];
      
      m_width = std::ceil(m_width / 50.f) * 50;
      m_height = std::ceil(m_height / 50.f) * 50;
      
      m_buffer = std::make_shared<BufferType>(m_width, m_height);
      
      m_buffer->ForEach([](const int& x, const int& y, KeyFrameItem& value)
                        {
                          value.pos = Vec2(x, y);
                        });
      
      for (const auto& item : json["region"])
      {
        PixelPos colorValue = item["c"];
        PixelPos x = item["x"];
        PixelPos y = item["y"];
        
        x -= 1;
        y -= 1;
        
        if (16777216 <= colorValue)
        {
          colorValue = colorValue;
        }
        
        if (colorValue != 0)
        {
          KeyFrameItem* bufferItem = nullptr;
          if (!m_buffer->Get(x, y, &bufferItem))
            return false;
          
          auto color = graphic::ColorFromUint(colorValue);
          bufferItem->color = color;
          bufferItem->id = nextId++;
        }
      }
      
      return true;
    }
    
    PixelPos m_width;
    PixelPos m_height;
    
    BufferTypePtr m_buffer;
    uint16_t nextId = 1;
  };
  
  class WorldModel
  {
  public:
    
    bool Init(const std::string& workingFolder)
    {
      m_workingFolder = workingFolder;
      
      std::string keyframeFileName = m_workingFolder + "/keyframe.json";
      if (!m_map.ReadFromFile(keyframeFileName))
      {
        return false;
      }
      
      if (!m_diffReader.Init(workingFolder))
      {
        return false;
      }
      
      inited = true;
      return true;
    }
    
    KeyFrameItem* GetItem(Vec2ConstRef pos) const
    {
      KeyFrameItem* result = nullptr;
      m_map.m_buffer->Get(pos.x, pos.y, &result);
      return result;
    }
    
    Vec2 GetSize() const
    {
      return Vec2(m_map.m_width, m_map.m_height);
    }
    
    void PlayUpdates(unsigned int numberOfUpdates, WorldModelDiffVect& updates)
    {
      if (!m_pendingDiffs.empty())
      {
        return PerformUpdates(numberOfUpdates, updates);
      }
      
      if (m_diffReader.IsAvailable())
      {
        assert(m_pendingDiffs.empty());
        m_diffReader.PopDiffs(m_pendingDiffs);
        
        if (!m_pendingDiffs.empty())
        {
          m_currentPosInDiffs = 0;
          return PerformUpdates(numberOfUpdates, updates);
        }
        else
        {
          m_diffReader.LoadNext();
        }
      }
    }
    
    void PerformUpdates(unsigned int numberOfUpdates, WorldModelDiffVect& result)
    {
      result.clear();
      result.reserve(numberOfUpdates);
      
      assert(!m_pendingDiffs.empty());
      size_t playableUpdats = std::min(static_cast<size_t>(numberOfUpdates), m_pendingDiffs.size() - m_currentPosInDiffs);
      
      for (unsigned int i = 0; i < playableUpdats; ++i)
      {
        unsigned int diffIndex = m_currentPosInDiffs + i;
        const auto& diff = m_pendingDiffs.at(diffIndex);
        
        auto soursePos = Vec2(diff.sourseX - 1, diff.sourseY - 1);
        auto destPos = Vec2(diff.destX - 1, diff.destY - 1);
        
        auto sourceItem = GetItem(soursePos);
        assert(sourceItem);
        auto destItem = GetItem(destPos);
        assert(destItem);
        
        if (soursePos == destPos)
        {
          if (diff.color == cocos2d::Color3B())
          {
            // assert(sourceItem->id != 0);
            if (sourceItem->id == 0)
            {
              continue;
            }
            
            Delete(sourceItem, result);
            continue;
          }
        }
        
        assert(diff.color != cocos2d::Color3B());
        //assert(soursePos != destPos);
        
        if (soursePos == destPos)
          continue;
        
        if (sourceItem->id == 0)
        {
          Create(diff.color, destItem, result);
          continue;
        }
        
        assert(sourceItem->id != 0);
        assert(destItem->id == 0);
        
        assert(diff.color == sourceItem->color);
        assert(!destItem->context);
        
        Move(sourceItem, destItem, result);
      }
      
      m_currentPosInDiffs += playableUpdats;
      if (m_currentPosInDiffs == m_pendingDiffs.size())
      {
        m_pendingDiffs.clear();
        m_currentPosInDiffs = 0;
      }
      
      m_updateId += 1;
    }
    
    void Move(KeyFrameItem* sourceItem, KeyFrameItem* destItem, WorldModelDiffVect& result)
    {
      assert(destItem);
      assert(sourceItem->id != 0);
      assert(destItem->id == 0);
      assert(!destItem->context);
      
      destItem->color = sourceItem->color;
      destItem->context = sourceItem->context;
      sourceItem->color = cocos2d::Color3B();
      sourceItem->context = nullptr;
      destItem->id = sourceItem->id;
      sourceItem->id = 0;
      
      WorldModelDiff resultDiff;
      resultDiff.id = destItem->id;
      resultDiff.context = destItem->context;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = destItem->pos;
      resultDiff.destinationItem = destItem;
      resultDiff.type = DiffType::Move;
      
      result.push_back(resultDiff);
    }
    
    void Delete(KeyFrameItem* sourceItem, WorldModelDiffVect& result)
    {
      assert(sourceItem->id != 0);
      
      ObjectContextPtr context = sourceItem->context;
      uint64_t id = sourceItem->id;
      
      sourceItem->color = cocos2d::Color3B();
      sourceItem->context = nullptr;
      sourceItem->id = 0;
      
      WorldModelDiff resultDiff;
      resultDiff.id = id;
      resultDiff.context = context;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = sourceItem->pos;
      resultDiff.destinationItem = sourceItem;
      resultDiff.type = DiffType::Delete;
      
      result.push_back(resultDiff);
    }
    
    void Create(cocos2d::Color3B color, KeyFrameItem* sourceItem, WorldModelDiffVect& result)
    {
      assert(sourceItem->id == 0);
      assert(sourceItem->color == cocos2d::Color3B());

      assert(sourceItem->context == nullptr);
      sourceItem->id = m_map.nextId++;
      sourceItem->color = color;
      
      WorldModelDiff resultDiff;
      resultDiff.id = sourceItem->id;
      resultDiff.context = nullptr;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = sourceItem->pos;
      resultDiff.destinationItem = sourceItem;
      resultDiff.type = DiffType::Add;
      
      result.push_back(resultDiff);
    }
    
    std::string m_workingFolder;
    KeyFrame m_map;
    bool inited = false;
    WorldModelDiffVect m_outputUpdates;
    AsyncDiffReader m_diffReader;
    DiffItemVector m_pendingDiffs;
    unsigned int m_currentPosInDiffs = 0;
    uint32_t m_updateId = 0;
  };
}
