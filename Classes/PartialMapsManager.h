//
//  PartialMapsManager.h
//  Komorki
//
//  Created 04.03.17.
//

#pragma once

#include <unordered_map>
#include <memory>
#include <list>
#include "Common.h"
#include "WorldModel.h"

namespace jevo
{
  namespace graphic
  {
    class ObjectContext;
    class PartialMap;

    using PartialMapPtr = std::shared_ptr<PartialMap>;
    using Maps = std::unordered_map<Vec2, PartialMapPtr, Vec2Hasher>;
    using ObjectContextPtr = std::shared_ptr<ObjectContext>;
    
    class PartialMapsManager
    {
    public:
      struct CreateMapArg
      {
        Rect rect;
        cocos2d::Vec2 graphicPos;
      };
      
      using CreateMapArgs = std::list<CreateMapArg>;
      using RemoveMapArgs = std::list<Vec2>;
      
      void Update(const CreateMapArgs& createMapArgs,
                  const RemoveMapArgs& mapsToRemove,
                  const WorldModelDiffVect& worldUpdate,
                  float animationDuration);
      void ProccessUpdate(const WorldModelDiff& diff, float animationDuration);
      void HealthCheck();
      
      const Maps& GetMaps() const;
      void EnableAnimation(bool enableAnimations, bool enableFancyAnimations);
      virtual ~PartialMapsManager();
      
    private:
      using GraphicContextList = std::list<ObjectContext*>;
      
      void PrintMap();
      PartialMapPtr GetMap(Vec2ConstRef pos);
      PartialMapPtr CreateMap(const CreateMapArg& args);
      ObjectContextPtr CreateObjectContext(GreatPixel* cell,
                                           Vec2ConstRef pos,
                                           const PartialMapPtr& map);
      void RemoveMap(const PartialMapPtr& map);

      void DeleteFromMap(GreatPixel* cd, const PartialMapPtr& map);
      void Move(ObjectContextPtr context,
                const Vec2& source,
                const Vec2& dest,
                const PartialMapPtr& map,
                int steps,
                float animationDuration);
      
      Maps m_map;


      // TODO: move the fields to constructor make them private
    public:
      cocos2d::Node* m_mainNode;
      cocos2d::Node* m_lightNode;
      std::shared_ptr<WorldModel> m_worldModel;
      Rect m_visibleArea; // visible area
      bool m_enableAnimations = false;
      bool m_enableFancyAnimaitons = false;
    };
  }
}

