
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
    class GraphicContext;
  }
  
  class AsyncKeyFrameReader;
  
  using GraphicContextPtr = std::shared_ptr<graphic::GraphicContext>;

  class GreatPixel;
  
  class Organizm
  {
  public:
    using Id = uint64_t;
    
    static const Organizm::Id UnknownOrgId = static_cast<Organizm::Id>(-1);
    static const Organizm::Id EnergyId = static_cast<Organizm::Id>(0);
    
    Organizm(Id id,
             GreatPixel* pos,
             cocos2d::Color3B color);
    virtual ~Organizm();
    void Move(GreatPixel* pos);
    void ChangeColor(cocos2d::Color3B color);
    void Delete();
    
    Id GetId() const;
    Vec2 GetPosition() const;
    const GraphicContextPtr& GetGraphicContext() const;
    void SetGraphicContext(const GraphicContextPtr& context);
    cocos2d::Color3B GetColor() const;
    uint64_t GetUpdateNumber() const;
    void SetUpdateNumber(uint64_t updateId);
    
    std::string Description() const;
    
  private:
    GraphicContextPtr m_context;
    cocos2d::Color3B m_color;
    Id m_id = UnknownOrgId;
    uint64_t m_updateNumber = 0;
    GreatPixel* m_pos;
  };
  

  
  using OrganizmPtr = std::shared_ptr<Organizm>;
  
  class GreatPixel
  {
  public:
    OrganizmPtr organizm;
    Vec2 pos;
  };
  
  using BufferType = Buffer2D<GreatPixel>;
  using BufferTypePtr = std::shared_ptr<BufferType>;
  
  enum class DiffType
  {
    Add,
    Move,
    Delete,
    Paint
  };
  
  class WorldModelDiff
  {
  public:
    DiffType type;
    OrganizmPtr organizm;
    GreatPixel* destinationPixel;
    Vec2 sourcePos;
    Vec2 destinationPos;
    
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
    
    void Move(Organizm::Id orgId,
              cocos2d::Color3B color,
              GreatPixel* sourceItem,
              GreatPixel* destItem,
              bool bypassResult,
              WorldModelDiffVect& result);
    void Delete(Organizm::Id orgId, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result);
    void Create(Organizm::Id orgId, cocos2d::Color3B color, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result);
    void Paint(Organizm::Id orgId, cocos2d::Color3B color, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result);
    
    std::string m_workingFolder;
    BufferTypePtr m_map;
    bool inited = false;
    WorldModelDiffVect m_outputUpdates;
    std::shared_ptr<AsyncDiffReader> m_diffReader;
    DiffItemVector m_pendingDiffs;
    unsigned int m_currentPosInDiffs = 0;
    uint32_t m_updateId = 1;
  };
}
