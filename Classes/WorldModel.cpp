

#include "WorldModel.h"
#include "ObjectContext.h"
#include "AsyncKeyFrameReader.h"

namespace jevo
{
  std::string WorldModelDiff::Description() const
  {
    std::stringstream ss;
    ss <<
    "[" <<
    "WorldModelDiff: " << static_cast<const void*>(this) <<
    " id: " << id <<
    " context: " << (context ? context->Description() : "null") <<
    " destinationItem: " << static_cast<const void*>(destinationPixel) <<
    " src: " << sourcePos.Description() <<
    " des: " << destinationPos.Description() <<
    "]";
    return ss.str();
  }
  
  bool WorldModel::Init(const std::string& workingFolder)
  {
    m_workingFolder = workingFolder;
    
    std::string keyframeFileName = m_workingFolder + "/keyframe.json";
    AsyncKeyFrameReader keyFrameReader;
    if (!keyFrameReader.ReadFromFile(keyframeFileName, m_nextId, m_map))
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
  
  bool WorldModel::Stop()
  {
    m_diffReader.Stop();
    return true;
  }
  
  GreatPixel* WorldModel::GetItem(Vec2ConstRef pos) const
  {
    GreatPixel* result = nullptr;
    m_map->Get(pos.x, pos.y, &result);
    return result;
  }
  
  Vec2 WorldModel::GetSize() const
  {
    return Vec2(m_map->GetWidth(), m_map->GetHeight());
  }
  
  void WorldModel::PlayUpdates(unsigned int numberOfUpdates, Rect visibleRect, WorldModelDiffVect& updates)
  {
    if (!m_pendingDiffs.empty())
    {
      return PerformUpdates(numberOfUpdates, visibleRect, updates);
    }
    
    if (m_diffReader.IsAvailable())
    {
      assert(m_pendingDiffs.empty());
      m_diffReader.PopDiffs(m_pendingDiffs);
      
      if (!m_pendingDiffs.empty())
      {
        m_currentPosInDiffs = 0;
        return PerformUpdates(numberOfUpdates, visibleRect, updates);
      }
      else
      {
        m_diffReader.LoadNext();
      }
    }
  }
  
  void WorldModel::PerformUpdates(unsigned int numberOfUpdates, Rect visibleRect, WorldModelDiffVect& result)
  {
    result.clear();
    result.reserve(numberOfUpdates);
    
    assert(!m_pendingDiffs.empty());
    size_t playableUpdats = m_pendingDiffs.size() - m_currentPosInDiffs;
    
    unsigned int i = 0;
    while(i < playableUpdats && result.size() < numberOfUpdates)
    {
      unsigned int diffIndex = m_currentPosInDiffs + i;
      const auto& diff = m_pendingDiffs.at(diffIndex);
      i += 1;
      
      auto soursePos = Vec2(diff.sourseX - 1, diff.sourseY - 1);
      auto destPos = Vec2(diff.destX - 1, diff.destY - 1);
      
      bool bypassResult = soursePos.In(visibleRect) || destPos.In(visibleRect);
      
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
          
          Delete(sourceItem, bypassResult, result);
          continue;
        }
      }
      
      assert(diff.color != cocos2d::Color3B());
      //assert(soursePos != destPos);
      
      if (soursePos == destPos)
        continue;
      
      if (sourceItem->id == 0)
      {
        Create(diff.color, destItem, bypassResult, result);
        continue;
      }
      
      assert(sourceItem->id != 0);
      assert(destItem->id == 0);
      
      assert(diff.color == sourceItem->color);
      assert(!destItem->context);
      
      Move(sourceItem, destItem, bypassResult, result);
    }
    
    m_currentPosInDiffs += i;
    if (m_currentPosInDiffs == m_pendingDiffs.size())
    {
      m_pendingDiffs.clear();
      m_currentPosInDiffs = 0;
    }
    
    m_updateId += 1;
  }
  
  void WorldModel::Move(GreatPixel* sourceItem, GreatPixel* destItem, bool bypassResult, WorldModelDiffVect& result)
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
    
    if (bypassResult)
    {
      WorldModelDiff resultDiff;
      resultDiff.id = destItem->id;
      resultDiff.context = destItem->context;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = destItem->pos;
      resultDiff.destinationPixel = destItem;
      resultDiff.type = DiffType::Move;
      
      result.push_back(resultDiff);
    }
  }
  
  void WorldModel::Delete(GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result)
  {
    assert(sourceItem->id != 0);
    
    ObjectContextPtr context = sourceItem->context;
    uint64_t id = sourceItem->id;
    
    sourceItem->color = cocos2d::Color3B();
    sourceItem->context = nullptr;
    sourceItem->id = 0;
    
    if (bypassResult)
    {
      WorldModelDiff resultDiff;
      resultDiff.id = id;
      resultDiff.context = context;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = sourceItem->pos;
      resultDiff.destinationPixel = sourceItem;
      resultDiff.type = DiffType::Delete;
      
      result.push_back(resultDiff);
    }
  }
  
  void WorldModel::Create(cocos2d::Color3B color, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result)
  {
    assert(sourceItem->id == 0);
    assert(sourceItem->color == cocos2d::Color3B());
    
    assert(sourceItem->context == nullptr);
    sourceItem->id = m_nextId++;
    sourceItem->color = color;
    
    if (bypassResult)
    {
      WorldModelDiff resultDiff;
      resultDiff.id = sourceItem->id;
      resultDiff.context = nullptr;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = sourceItem->pos;
      resultDiff.destinationPixel = sourceItem;
      resultDiff.type = DiffType::Add;
      
      result.push_back(resultDiff);
    }
  }
}
