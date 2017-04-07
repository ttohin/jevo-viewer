//
//  PartialMapsManager.cpp
//  Komorki
//
//  Created on 04.03.17.
//

#include "PartialMapsManager.h"
#include "GraphicContext.h"
#include "PartialMap.h"
#include "SharedUIData.h"
#include "UIConfig.h"
#include "Logging.h"
#include "SharedUIData.h"
#include "SpriteBatch.h"

namespace jevo
{
  namespace graphic
  {
    //********************************************************************************************
    Vec2 GetMapOriginFromPos(const Vec2& pos)
    {
      return Vec2((pos.x/kSegmentSize) * kSegmentSize,
                  (pos.y/kSegmentSize) * kSegmentSize);
    }
    
    //********************************************************************************************
    void PartialMapsManager::Update(const CreateMapArgs &createMapArgs,
                                    const RemoveMapArgs& mapsToRemove,
                                    const WorldModelDiffVect &worldUpdate,
                                    float animationDuration)
    {
      for (auto& m : mapsToRemove)
      {
        
        auto it = m_map.find(GetMapOriginFromPos(m));
        assert(it != m_map.end());
        LOG_W("PartialMapsManager::Update. Delete maps. Map: %s", it->second->Description().c_str());
        RemoveMap(it->second);
        m_map.erase(it->first);
      }
      
      for (const auto& createMapArg : createMapArgs)
      {
        auto map = CreateMap(createMapArg);
      }
      
      for (const auto& u : worldUpdate)
      {
        ProccessUpdate(u, animationDuration);
      }
      
      HealthCheck();
    }
    
    //********************************************************************************************
    void PartialMapsManager::ProccessUpdate(const WorldModelDiff& u, float animationDuration)
    {
      Vec2 initialPos = u.sourcePos;
      
      DiffType type = u.type;
      
      Vec2 destinationPos = initialPos;
      
      OrganizmPtr organizm = u.organizm;
      
      assert(organizm);
      
      auto context = organizm->GetGraphicContext();
      
      if(type == DiffType::Move)
      {
        if (!context)
          return;
        
        destinationPos = u.destinationPos;
      }
      
      PartialMapPtr initialMap = GetMap(GetMapOriginFromPos(initialPos));
      PartialMapPtr destinationMap = GetMap(GetMapOriginFromPos(destinationPos));
      
      if (!initialMap && !destinationMap)
      {
        if (context)
        {
          DeleteFromMap(organizm);
        }
        return;
      }
      
      if (!destinationMap)
      {
        assert(context);
        DeleteFromMap(organizm);
        return;
      }
      
      if (type == DiffType::Move)
      {
        PartialMapPtr mapForAction = destinationMap ? destinationMap : initialMap;
        
        // create context if it's needed
        if (!context)
        {
          context = CreateGraphicContext(u.destinationPixel,
                                        initialPos,
                                        mapForAction);
        }
        
        assert(context);
        
        int steps = 0;
        Move(context,
             initialPos,
             destinationPos,
             mapForAction,
             steps,
             animationDuration);
        
        context->FadeCell();
        
        return;
      }
      
      if (type == DiffType::Add)
      {
        assert(context == nullptr);
        PartialMapPtr mapForAction = destinationMap ? destinationMap : initialMap;
        context = CreateGraphicContext(u.destinationPixel,
                                      destinationPos,
                                      mapForAction);
        assert(context);
        if (organizm->GetId() == 0) context->Alert(cocos2d::Color3B::GREEN);
        if (organizm->GetId() != 0) context->Alert(cocos2d::Color3B::YELLOW);
      }
      
      if (type == DiffType::Delete)
      {
        if (!context)
        {
          return;
        }
        
        assert(context);
        context->FadeCell();
        if (organizm->GetId() == 0) context->Alert(cocos2d::Color3B::BLUE);
        if (organizm->GetId() != 0) context->Alert(cocos2d::Color3B::RED);
        DeleteFromMap(organizm);
      }
    }
    
    //********************************************************************************************
    void PartialMapsManager::HealthCheck()
    {
      if (!config::healthCheck)
        return;
      
      auto size = m_worldModel->GetSize();
      for (int i = 0; i < size.x; ++i)
      {
        for (int j = 0; j < size.y; ++j)
        {
          auto pos = Vec2(i, j);
          
          auto pd = m_worldModel->GetItem(pos);
          assert(pd);
          if (!pos.In(m_visibleArea) && pd->organizm)
          {
            assert(!pd->organizm->GetGraphicContext());
          }
        }
      }
      
      auto instanceCounter = PartialMap::instanceCounter;
      assert(instanceCounter == m_map.size());
    }
    
    //********************************************************************************************
    PartialMapPtr PartialMapsManager::CreateMap(const CreateMapArg& args)
    {
      auto map = std::make_shared<graphic::PartialMap>();
      map->Init(args.rect.origin.x,
                args.rect.origin.y,
                args.rect.size.x,
                args.rect.size.y,
                m_mainNode,
                m_lightNode,
                cocos2d::Vec2::ZERO);

      map->Transfrorm(args.graphicPos, 1.0);
      map->EnableAnimations(m_enableAnimations);
      map->EnableFancyAnimations(m_enableFancyAnimaitons);
      
      auto it = m_map.insert(std::make_pair(GetMapOriginFromPos(args.rect.origin), map));
      assert(it.second);
      
      for (int i = map->m_a1; i < map->m_a2; ++i)
      {
        for (int j = map->m_b1; j < map->m_b2; ++j)
        {
          auto pos = Vec2(i, j);
          auto pd = m_worldModel->GetItem(pos);
          assert(pd);
          CreateGraphicContext(pd, pos, map);
        }
      }
      
      LOG_W("PartialMapsManager::CreateMap. %s", map->Description().c_str());
      
      return map;
    }
    
    //********************************************************************************************
    const Maps& PartialMapsManager::GetMaps() const
    {
      return m_map;
    }
    
    //********************************************************************************************
    PartialMapPtr PartialMapsManager::GetMap(Vec2ConstRef pos)
    {
      auto it = m_map.find(pos);
      if (m_map.end() == it)
        return nullptr;
      else
        return it->second;
    }
    
    //********************************************************************************************
    void PartialMapsManager::EnableAnimation(bool enableAnimations, bool enableFancyAnimations)
    {
      if (m_enableAnimations == enableAnimations &&
          m_enableFancyAnimaitons == enableFancyAnimations)
        return;
      
      m_enableAnimations = enableAnimations;
      m_enableFancyAnimaitons = enableFancyAnimations;
      
      for (auto& m : m_map)
      {
        m.second->EnableAnimations(m_enableAnimations);
        m.second->EnableFancyAnimations(m_enableFancyAnimaitons);
      }
      
      for (int i = m_visibleArea.origin.x; i < m_visibleArea.origin.x + m_visibleArea.size.x; ++i)
      {
        for (int j = m_visibleArea.origin.y; j < m_visibleArea.origin.y + m_visibleArea.size.y; ++j)
        {
          auto pos = Vec2(i, j);
          const GraphicContextPtr& graphicContext = GetGraphicContext(pos);
          if (graphicContext)
          {
            graphicContext->ToggleAnimation();
          }
        }
      }
    }
    
    //********************************************************************************************
    PartialMapsManager::~PartialMapsManager()
    {
      auto contextInstanceCount = GraphicContext::instanceCounter;
      
      auto deletedContext = 0;
      
      auto size = m_worldModel->GetSize();
      for (int i = 0; i < size.x; ++i)
      {
        for (int j = 0; j < size.y; ++j)
        {
          auto pos = Vec2(i, j);
          auto pixel = m_worldModel->GetItem(pos);
          auto organizm = pixel->organizm;
          
          if (!organizm || !organizm->GetGraphicContext()) continue;
          
          organizm->SetGraphicContext(nullptr);
          deletedContext += 1;
        }
      }
      
      auto contextInstanceCountAfterRemoval = GraphicContext::instanceCounter;
      assert(contextInstanceCountAfterRemoval == 0);
      assert(deletedContext == contextInstanceCount);
      
      m_map.clear();
      
      auto partialMapsCount = PartialMap::instanceCounter;
      assert(partialMapsCount == 0);
      
      auto mainNodeChildrenCount = m_mainNode->getChildren().size();
      assert(mainNodeChildrenCount == 0);
      
      auto lightNodeChildrenCount = m_lightNode->getChildren().size();
      assert(lightNodeChildrenCount == 0);
      
      graphic::SharedUIData::getInstance()->m_textureMap.clear();
      
      auto sprites = SpriteBatch::instanceCounter;
      assert(sprites == 0);
    }
    
    //********************************************************************************************
    GraphicContextPtr PartialMapsManager::CreateGraphicContext(GreatPixel* pixel,
                                                               Vec2ConstRef pos,
                                                               const PartialMapPtr& map)
    {
      const OrganizmPtr& organizm = pixel->organizm;
      
      if (!organizm)
        return nullptr;
      
      GraphicContextPtr context = organizm->GetGraphicContext();
      if (context)
      {
        context->BecomeOwner(map);
        context->tt_pos = pos;
        return context;
      }
      
      auto textureRect = cocos2d::Rect(0, 0, 1, 1);
      
      auto rect = Rect(pos, Vec2(1, 1));
      context = std::make_shared<GraphicContext>(organizm->GetId(),
                                                 map,
                                                 organizm->GetColor(),
                                                 textureRect,
                                                 pos,
                                                 rect);
      organizm->SetGraphicContext(context);
      context->tt_pos = pos;
      context->ToggleAnimation();
      
      return context;
    }
    
    //********************************************************************************************
    GraphicContextPtr PartialMapsManager::GetGraphicContext(Vec2 pos) const
    {
      auto pd = m_worldModel->GetItem(pos);
      assert(pd);
      return pd->organizm ? pd->organizm->GetGraphicContext() : nullptr;
    }
    
    //********************************************************************************************
    void PartialMapsManager::RemoveMap(const PartialMapPtr& map)
    {
      for (int i = map->m_a1; i < map->m_a2; ++i)
      {
        for (int j = map->m_b1; j < map->m_b2; ++j)
        {
          auto pos = Vec2(i, j);
          auto pixel = m_worldModel->GetItem(pos);
          auto organizm = pixel->organizm;
          
          if (!organizm) continue;
          
          auto graphicContext = organizm->GetGraphicContext();
          
          if (graphicContext)
          {
            graphicContext->Destory();
            organizm->SetGraphicContext(nullptr);
          }
        }
      }
    }

    //********************************************************************************************
    void PartialMapsManager::DeleteFromMap(const OrganizmPtr& organizm)
    {
      const GraphicContextPtr& graphicContext = organizm->GetGraphicContext();
      
      if (graphicContext == nullptr)
        return;
      
      graphicContext->Destory();
      organizm->SetGraphicContext(nullptr);
    }
    
    //********************************************************************************************
    void PartialMapsManager::Move(GraphicContextPtr context,
                                  const Vec2& source,
                                  const Vec2& dest,
                                  const PartialMapPtr& map,
                                  int steps,
                                  float animationDuration)
    {
      assert(context);
      
      context->BecomeOwner(map);
      
      context->tt_pos = dest;
      context->Move(source, dest, animationDuration*0.9*(steps + 1));
    }
  }
}
