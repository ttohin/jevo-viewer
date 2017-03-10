//
//  PartialMapsManager.cpp
//  Komorki
//
//  Created on 04.03.17.
//

#include "PartialMapsManager.h"
#include "ObjectContext.h"
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
      static int counter = 0;
      std::cout << "ttnum " << counter << std::endl;
      std::cout << m_visibleArea.Description() << std::endl;
      counter += 1;
      
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
        std::cout << u.Description() << std::endl;
        
        Vec2 initialPos = u.sourcePos;
        
        DiffType type = u.type;
        
        Vec2 destinationPos = initialPos;
        
        ObjectContextPtr context = u.context;
        
        if (context && u.destinationItem->context)
        {
          assert(context == u.destinationItem->context);
        }
        
        if(type == DiffType::Move)
        {
          if (!context)
            continue;
          
          assert(u.context == u.destinationItem->context);
          
          destinationPos = u.destinationPos;
        }
        
        PartialMapPtr initialMap = GetMap(GetMapOriginFromPos(initialPos));
        PartialMapPtr destinationMap = GetMap(GetMapOriginFromPos(destinationPos));
        
        if (!initialMap && !destinationMap)
        {
          assert(!context);
          continue;
        }
        
        if (!destinationMap)
        {
          assert(context);
          LOG_W("cell is going to outside %s %s %s", __FUNCTION__, destinationPos.Description().c_str(), u.destinationItem->context->Description().c_str());
          DeleteFromMap(u.destinationItem, initialMap);
          continue;
        }
        
        if (type == DiffType::Move)
        {
          PartialMapPtr mapForAction = destinationMap ? destinationMap : initialMap;
          
          // create context if it's needed
          if (!context)
          {
            context = CreateObjectContext(u.destinationItem,
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
          
          continue;
        }
        
        
        if (type == DiffType::Delete)
        {
          if (!context)
          {
            continue;
          }
          
          assert(context);
          context->CellDead();
          DeleteFromMap(u.destinationItem, initialMap);
        }
        
      }
      
      auto size = m_worldModel->GetSize();
      for (int i = 0; i < size.x; ++i)
      {
        for (int j = 0; j < size.y; ++j)
        {
          auto pos = Vec2(i, j);
          
          auto pd = m_worldModel->GetItem(pos);
          assert(pd);
          if (!pos.In(m_visibleArea))
          {
            assert(!pd->context);
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

      map->Transfrorm(args.graphicPos,
                      1.0);
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
          CreateObjectContext(pd, pos, map);
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
          auto pd = m_worldModel->GetItem(pos);
          assert(pd);
          if (pd->context)
          {
            pd->context->ToggleAnimation();
          }
        }
      }
    }
    
    PartialMapsManager::~PartialMapsManager()
    {
      auto contextInstanceCount = ObjectContext::instanceCounter;
      
      auto deletedContext = 0;
      
      auto size = m_worldModel->GetSize();
      for (int i = 0; i < size.x; ++i)
      {
        for (int j = 0; j < size.y; ++j)
        {
          auto pos = Vec2(i, j);
          auto pd = m_worldModel->GetItem(pos);
          assert(pd);
          if (pd->context)
          {
            pd->context = nullptr;
            deletedContext += 1;
          }
        }
      }
      
      auto contextInstanceCountAfterRemoval = ObjectContext::instanceCounter;
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
    ObjectContextPtr PartialMapsManager::CreateObjectContext(KeyFrameItem* cell,
                                                             Vec2ConstRef pos,
                                                             const PartialMapPtr& map)
    {
      if (cell->context)
      {
        cell->context->BecomeOwner(map);
        return cell->context;
      }
      
      if (cell->color == cocos2d::Color3B())
        return nullptr;
      
      //TODO: get texture from file
//      auto groupId = 0;
//      auto textureRect = SharedUIData::getInstance()->m_textureMap[groupId];
      auto textureRect = cocos2d::Rect(0, 0, 1, 1);
      
      auto rect = Rect(pos, Vec2(1, 1));
      cell->context = std::make_shared<ObjectContext>(map,
                                                    cell->color,
                                                    textureRect,
                                                    pos,
                                                    rect);
      cell->context->ToggleAnimation();
      return cell->context;
    }
    
    void PartialMapsManager::PrintMap()
    {
      auto size = m_worldModel->GetSize();
      for (int i = 0; i < size.x; ++i)
      {
        for (int j = 0; j < size.y; ++j)
        {
          auto pos = Vec2(i, j);
          
          auto pd = m_worldModel->GetItem(pos);
          assert(pd);
          if (pd->context)
            LOG_W("%s %s %s", __FUNCTION__, pos.Description().c_str(), pd->context->Description().c_str());
          else
            LOG_W("%s %s null", __FUNCTION__, pos.Description().c_str());
        }
      }
    }
    
    void PartialMapsManager::RemoveMap(const PartialMapPtr& map)
    {
      
      
      for (int i = map->m_a1; i < map->m_a2; ++i)
      {
        for (int j = map->m_b1; j < map->m_b2; ++j)
        {
          auto pos = Vec2(i, j);
          auto pd = m_worldModel->GetItem(pos);
          assert(pd);
          if (pd->context)
          {
            pd->context->Destory(nullptr);
            pd->context = nullptr;
          }
        }
      }
      
      if(!map->m_contexts.empty())
      {
        PrintMap();
        assert(0);
      }
    }

    //********************************************************************************************
    void PartialMapsManager::DeleteFromMap(KeyFrameItem* cd,
                                           const PartialMapPtr& map)
    {
      if (cd->context == nullptr)
        return;
      
      cd->context->Destory(map);
      cd->context = nullptr;
    }
    
    //********************************************************************************************
    void PartialMapsManager::Move(ObjectContextPtr context,
                                  const Vec2& source,
                                  const Vec2& dest,
                                  const PartialMapPtr& map,
                                  int steps,
                                  float animationDuration)
    {
      assert(context);
      
      context->tt_pos = dest;
      context->Move(source, dest, animationDuration*0.9*(steps + 1));
    }
  }
}
