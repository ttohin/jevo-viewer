
#pragma once

#include <fstream>
#include <sstream>
#include "json_safe.hpp"
#include "UICommon.h"
#include "Buffer2D.h"
#include "AsyncDiffReader.h"
#include <algorithm>

namespace jevo
{
  namespace graphic
  {
    class ObjectContext;
  }
  
  class AsyncKeyFrameReader;
  
  using ObjectContextPtr = std::shared_ptr<graphic::ObjectContext>;
  using CellId = uint64_t;
  
  class GreatPixel
  {
  public:
    cocos2d::Color3B color;
    ObjectContextPtr context;
    CellId id;
    Vec2 pos;
  };
  
  using BufferType = Buffer2D<GreatPixel>;
  using BufferTypePtr = std::shared_ptr<BufferType>;
  
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
    GreatPixel* destinationPixel;
    Vec2 sourcePos;
    Vec2 destinationPos;
    CellId id;
    
    std::string Description() const;
  };
  
  using WorldModelDiffVect = std::vector<WorldModelDiff>;
  
  class WorldModel
  {
  public:
    
    bool Init(const std::string& workingFolder);
    bool Stop();
    GreatPixel* GetItem(Vec2ConstRef pos) const;
    Vec2 GetSize() const;
    void PlayUpdates(unsigned int numberOfUpdates, Rect visibleRect, WorldModelDiffVect& updates);
    void PerformUpdates(unsigned int numberOfUpdates, Rect visibleRect, WorldModelDiffVect& result);
    
    void Move(GreatPixel* sourceItem, GreatPixel* destItem, bool bypassResult, WorldModelDiffVect& result);
    void Delete(GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result);
    void Create(cocos2d::Color3B color, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result);
    
    std::string m_workingFolder;
    BufferTypePtr m_map;
    bool inited = false;
    WorldModelDiffVect m_outputUpdates;
    AsyncDiffReader m_diffReader;
    DiffItemVector m_pendingDiffs;
    unsigned int m_currentPosInDiffs = 0;
    uint32_t m_updateId = 0;
    CellId m_nextId = 1;
  };
}
