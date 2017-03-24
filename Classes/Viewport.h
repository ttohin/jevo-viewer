#pragma once

#include <memory>
#include "cocos2d.h"
#include "Common.h"
#include "WorldModel.h"
#include "PartialMapsManager.h"

namespace jevo
{
  class WorldModel;

  namespace graphic
  {
    class PartialMap;

    class Viewport
    {
    public:

      typedef std::shared_ptr<Viewport> Ptr;

      void Move(const cocos2d::Vec2& ofset);
      void Zoom(const cocos2d::Vec2& point, float scaleOffset);
      void Calculate();

      Viewport(cocos2d::Node* superView,
               const cocos2d::Size& originalSize,
               const std::shared_ptr<jevo::WorldModel>& worldModel);

      ~Viewport();
      void Test();
      void CreateMap();
      void Resize(const cocos2d::Size& originalSize);

      void Update(float updateTime, float& outUpdateTime);
      void UpdateAsync(float& updateTime);
      bool IsAvailable();
      bool Destroy();
      cocos2d::Node* GetRootNode() const;
      cocos2d::Node* GetMainNode() const;
      cocos2d::Node* GetLightNode() const;

    private:

      typedef std::shared_ptr<PartialMap> PartialMapPtr;
      typedef std::vector<PartialMapPtr> MapList;

      void PerformMove(PartialMapsManager::CreateMapArgs& newMapsArgs,
                       PartialMapsManager::RemoveMapArgs& mapsToRemove);
      void CreateMap(const cocos2d::Rect& viewSize, float scale);
      void CreatePixelMaps(const Rect& rect, const cocos2d::Vec2& offset, float scale);

      bool RemoveMapsOutsideOfRect(const Rect& rect, const Maps& currentMaps, PartialMapsManager::RemoveMapArgs& mapsToRemove);
      bool SplitRectOnChunks(const Rect& rect, const Rect& existingRect, std::vector<Rect>& result) const;
      bool FillCreateMapsArgs(const std::vector<Rect>& rects,
                              PartialMapsManager::CreateMapArgs& newMapsArgs);
      cocos2d::Rect GetCurrentGraphicRect() const;

      int m_mapSegmentSize;

      cocos2d::Node* m_mainView;
      cocos2d::Node* m_superView;
      cocos2d::Node* m_lightNode;

      bool m_performMove;

      std::shared_ptr<jevo::WorldModel> m_worldModel;
      WorldModelDiffVect m_worldUpdateResult;
      PartialMapsManager m_mapManager;

      Rect tt_loadedPixelRect;
      cocos2d::Size tt_viewSize;
    };
  }
}

