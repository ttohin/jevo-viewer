

#include "WorldModel.h"
#include "GraphicContext.h"
#include "AsyncKeyFrameReader.h"

namespace jevo
{
  Organizm::Organizm(GreatPixel* pos, Organizm::Id id, cocos2d::Color3B color)
  : m_context(nullptr)
  , m_color(color)
  , m_id(id)
  , m_pos(pos)
  {
    assert(m_pos);
    assert(!m_pos->organizm);
    assert(color != cocos2d::Color3B());
    assert(m_id != UnknownOrgId);
  }
  
  Organizm::~Organizm()
  {
    assert(m_pos->organizm.get() != this);
  }
  
  void Organizm::Move(GreatPixel* pos)
  {
    assert(pos);
    assert(!pos->organizm);
    assert(pos != m_pos);
    assert(m_pos->organizm.get() == this);
    
    pos->organizm = m_pos->organizm;
    m_pos->organizm = nullptr;
    
    m_pos = pos;
  }
  
  void Organizm::ChangeColor(cocos2d::Color3B color)
  {
    m_color = color;
  }
  
  void Organizm::Delete()
  {
    m_pos->organizm = nullptr;
  }
  
  Organizm::Id Organizm::GetId() const
  {
    return m_id;
  }
  
  Vec2 Organizm::GetPosition() const
  {
    return m_pos->pos;
  }
  
  const GraphicContextPtr& Organizm::GetGraphicContext() const
  {
    return m_context;
  }
  
  void Organizm::SetGraphicContext(const GraphicContextPtr& context)
  {
    m_context = context;
  }
  
  cocos2d::Color3B Organizm::GetColor() const
  {
    return m_color;
  }
  
  std::string Organizm::Description() const
  {
    std::stringstream ss;
    ss <<
    "[" <<
    "WorldModelDiff: " << static_cast<const void*>(this) <<
    " id: " << m_id <<
    " context: " << (m_context ? m_context->Description() : "null") <<
    " pos: " << GetPosition().Description() <<
    "]";
    return ss.str();
  }
  
  std::string WorldModelDiff::Description() const
  {
    std::stringstream ss;
    ss <<
    "[" <<
    "WorldModelDiff: " << static_cast<const void*>(this) <<
    " organizm: " << (organizm ? organizm->Description() : "null") <<
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
    if (!keyFrameReader.ReadFromFile(keyframeFileName, m_map))
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
      
      Organizm::Id OrgId = diff.id == 0 ? EnergyId : diff.id;
      
      if (diff.action == "delete")
      {
        Delete(OrgId, sourceItem, bypassResult, result);
      }
      else if (diff.action == "add")
      {
        assert(diff.color != cocos2d::Color3B());
        Create(OrgId, diff.color, destItem, bypassResult, result);
      }
      else if (diff.action == "move")
      {
        Move(OrgId, diff.color, sourceItem, destItem, bypassResult, result);
      }
    }
    
    m_currentPosInDiffs += i;
    if (m_currentPosInDiffs == m_pendingDiffs.size())
    {
      m_pendingDiffs.clear();
      m_currentPosInDiffs = 0;
    }
    
    m_updateId += 1;
  }
  
  void WorldModel::Move(Organizm::Id orgId,
                        cocos2d::Color3B color,
                        GreatPixel* sourceItem,
                        GreatPixel* destItem,
                        bool bypassResult,
                        WorldModelDiffVect& result)
  {
    assert(destItem);
    assert(sourceItem);
    
    OrganizmPtr organizm = sourceItem->organizm;
    
    // hack
    // assert(organizm);
    if (!organizm)
    {
      return Create(orgId, color, destItem, bypassResult, result);
    }
    
    assert(organizm->GetId() == orgId);
    
    organizm->Move(destItem);
    
    if (bypassResult)
    {
      WorldModelDiff resultDiff;
      resultDiff.organizm = organizm;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = destItem->pos;
      resultDiff.destinationPixel = destItem;
      resultDiff.type = DiffType::Move;
      
      result.push_back(resultDiff);
    }
  }
  
  void WorldModel::Delete(Organizm::Id orgId, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result)
  {
    assert(sourceItem);
    
    OrganizmPtr organizm = sourceItem->organizm;
    
    // hack
    // assert(organizm);
    
    if (!organizm)
    {
      return;
    }
    
    organizm->Delete();
    
    if (bypassResult)
    {
      WorldModelDiff resultDiff;
      resultDiff.organizm = organizm;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = sourceItem->pos;
      resultDiff.destinationPixel = sourceItem;
      resultDiff.type = DiffType::Delete;
      
      result.push_back(resultDiff);
    }
  }
  
  void WorldModel::Create(Organizm::Id orgId, cocos2d::Color3B color, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result)
  {
    assert(sourceItem);
    
    auto organizm = std::make_shared<Organizm>(sourceItem, orgId, color);
    sourceItem->organizm = organizm;
    
    if (bypassResult)
    {
      WorldModelDiff resultDiff;
      resultDiff.organizm = organizm;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = sourceItem->pos;
      resultDiff.destinationPixel = sourceItem;
      resultDiff.type = DiffType::Add;
      
      result.push_back(resultDiff);
    }
  }
  
  void WorldModel::Paint(Organizm::Id orgId, cocos2d::Color3B color, GreatPixel* sourceItem, bool bypassResult, WorldModelDiffVect& result)
  {
    assert(sourceItem);
    
    OrganizmPtr organizm = sourceItem->organizm;
    // hack
    // assert(organizm);
    if (!organizm)
    {
      return Create(orgId, color, sourceItem, bypassResult, result);
    }
    
    organizm->ChangeColor(color);
    
    if (bypassResult)
    {
      WorldModelDiff resultDiff;
      resultDiff.organizm = organizm;
      resultDiff.sourcePos = sourceItem->pos;
      resultDiff.destinationPos = sourceItem->pos;
      resultDiff.destinationPixel = sourceItem;
      resultDiff.type = DiffType::Paint;
      
      result.push_back(resultDiff);
    }
  }
}
