

#include "Viewport.h"
#include "PartialMap.h"
#include "UIConfig.h"
#include "UICommon.h"
#include "Logging.h"


namespace jevo
{
  namespace graphic
  {
    Rect TTPixelRect(const cocos2d::Rect& rect)
    {
      Rect result;

      result.origin.x = std::floor(rect.origin.x/(kSpritePosition));
      result.size.x = std::ceil(rect.size.width/(kSpritePosition));
      result.origin.y = std::floor(rect.origin.y/(kSpritePosition));
      result.size.y = std::ceil(rect.size.height/(kSpritePosition));

      return result;
    }

    PixelPos Ceil(const PixelPos& value, const PixelPos& precision)
    {
      double scaledValue = (double) value / precision;
      return std::ceil(scaledValue) * precision;
    }

    Rect ResizeByStep(const Rect& original, const Rect& destination, const PixelPos& step)
    {
      Rect result = original;
      if (original.Top() > destination.Top())
      {
        PixelPos offset = original.Top() - destination.Top();
        result.size.y -= Ceil(offset, step);
      }
      if (original.Bottom() < destination.Bottom())
      {
        PixelPos offset = destination.Bottom() - original.Bottom();
        result.MoveBotton(Ceil(offset, step));
      }
      if (original.Left() < destination.Left())
      {
        PixelPos offset = destination.Left() - original.Left();
        result.MoveLeft(Ceil(offset, step));
      }
      if (original.Right() > destination.Right())
      {
        PixelPos offset = original.Right() - destination.Right();
        result.size.x -= Ceil(offset, step);
      }
      return result;
    }

    Rect ExtendRectWithStep(const Rect& original, const Rect& destination, const PixelPos& step)
    {
      Rect result = original;
      if (original.Top() < destination.Top())
      {
        PixelPos offset = destination.Top() - original.Top();
        result.size.y += Ceil(offset, step);
      }
      if (original.Bottom() > destination.Bottom())
      {
        PixelPos offset = original.Bottom() - destination.Bottom();
        result.MoveBotton(-Ceil(offset, step));
      }
      if (original.Left() > destination.Left())
      {
        PixelPos offset = original.Left() - destination.Left();
        result.MoveLeft(-Ceil(offset, step));
      }
      if (original.Right() < destination.Right())
      {
        PixelPos offset = destination.Right() - original.Right();
        result.size.x += Ceil(offset, step);
      }
      return result;
    }

    void Viewport::Move(const cocos2d::Vec2& offset)
    {
      cocos2d::Vec2 newpos = m_superView->getPosition() + offset;
      m_superView->setPosition(newpos);
    }

    void Zoom(const cocos2d::Vec2& base,
              const cocos2d::Vec2& point,
              const float& scale,
              const float& scaleOffset,
              cocos2d::Vec2& outBase,
              float& outScale)
    {
      float newScale = scale - scaleOffset;
      float scaleRatio = newScale/scale;

      outScale = newScale;

      outBase.x = point.x  - scaleRatio * (point.x -  base.x);
      outBase.y = point.y  - scaleRatio * (point.y - base.y);
    }

    void Viewport::Zoom(const cocos2d::Vec2& point, float scaleOffset)
    {
      cocos2d::Vec2 newSuperViewPos;
      float newSuperViewScale;
      graphic::Zoom(m_superView->getPosition(), point, m_superView->getScale(), scaleOffset, newSuperViewPos, newSuperViewScale);
      
      if (newSuperViewScale < 0.02 || newSuperViewScale > 0.6)
        return;
      
      m_superView->setPosition(newSuperViewPos);
      m_superView->setScale(newSuperViewScale);
    }

    void Viewport::Calculate()
    {
      auto graphicalVisibleRect = GetCurrentGraphicRect();

      Rect pixelRect = TTPixelRect(graphicalVisibleRect);
      Rect innerPrevRect = ResizeByStep(tt_loadedPixelRect, pixelRect, kSegmentSize);
      Rect extendedRect = ExtendRectWithStep(innerPrevRect, pixelRect, kSegmentSize);
      Vec2 size = m_worldModel->GetSize();
      extendedRect = extendedRect.Extract({Vec2(0, 0), size});

      if ( extendedRect != tt_loadedPixelRect && extendedRect.size != Vec2())
      {
        m_performMove = true;
      }
    }

    Viewport::Viewport(cocos2d::Node* superView,
                       const cocos2d::Size& originalSize,
                       const std::shared_ptr<WorldModel>& worldModel)

    {
      assert(superView);

      tt_viewSize = originalSize;
      m_superView = superView;
      m_superView->setScale(config::initialScale);

      m_lightNode = cocos2d::Node::create();
      m_mainView = cocos2d::Node::create();
      m_superView->addChild(m_lightNode);
      m_superView->addChild(m_mainView);
      m_performMove = false;

      m_worldModel = worldModel;

      m_mapManager.m_mainNode = m_mainView;
      m_mapManager.m_lightNode = m_lightNode;
      m_mapManager.m_worldModel = worldModel;
      m_mainView->setName("SuperView");

      CreateMap();

      Test();
    }

    Viewport::~Viewport()
    {
      Destroy();
    }

    void Viewport::Test()
    {
      {
        Rect r1 = {{0,0}, {60,70}};
        Rect existingRect = {{0,0}, {50,50}};
        Rect expected2 = {{50,0}, {10,50}};
        Rect expected3 = {{0,50}, {50,20}};
        Rect expected4 = {{50,50}, {10,20}};

        std::vector<Rect> res;
        assert(jevo::SplitRectOnChunks(r1, existingRect, 50, res));
        assert(res.size() == 3);
        assert(res.end() != std::find(res.begin(), res.end(), expected2));
        assert(res.end() != std::find(res.begin(), res.end(), expected3));
        assert(res.end() != std::find(res.begin(), res.end(), expected4));

      }

      {
        Rect r1 = {{0,0}, {100,100}};
        Rect r2 = {{-10,10}, {120,60}};
        Rect expected = {{0,20}, {100,40}};
        Rect r = ResizeByStep(r1, r2, 20);
        assert(r == expected);
      }

      {
        Rect r1 = {{2,2}, {2,2}};
        Rect r2 = {{0,0}, {10,10}};
        Rect expected = {{2,2}, {2,2}};
        Rect r = ResizeByStep(r1, r2, 10);
        assert(r == expected);
      }

      {
        Rect r1 = {{0,0}, {30,30}};
        Rect r2 = {{-5,-15}, {40,50}};
        Rect expected = {{-10,-20}, {50,60}};
        Rect r = ExtendRectWithStep(r1, r2, 10);
        assert(r == expected);
      }

      {
        float f1 = 10.1;
        int r1 = std::ceil(f1);
        assert(r1 == 11);
      }
      {
        float f1 = 10.1;
        int r1 = std::floor(f1);
        assert(r1 == 10);
      }

      {
        float f1 = 10.9;
        int r1 = std::ceil(f1);
        assert(r1 == 11);
      }
      {
        float f1 = 10.9;
        int r1 = std::floor(f1);
        assert(r1 == 10);
      }

      {
        Rect r1 = {{0,0}, {10,10}};
        Rect r2 = {{0,0}, {20,20}};
        assert(r1.In(r2));
      }

      {
        Rect r1 = {{0,0}, {10,10}};
        Rect r2 = {{-4,-2}, {20,20}};
        assert(r1.In(r2));
      }

      {
        Rect r1 = {{0,0}, {10,10}};
        Rect r2 = {{0,0}, {20,20}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{0,0}, {10,10}}));
      }
      {
        Rect r1 = {{-1,-1}, {10,10}};
        Rect r2 = {{0,0}, {20,20}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{0,0}, {9,9}}));
      }
      {
        Rect r1 = {{10,10}, {10,10}};
        Rect r2 = {{0,0}, {20,20}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{10,10}, {10,10}}));
      }
      {
        Rect r1 = {{15,15}, {10,10}};
        Rect r2 = {{0,0}, {20,20}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{15,15}, {5,5}}));
      }
      {
        Rect r1 = {{0,0}, {10,10}};
        Rect r2 = {{5,5}, {2,2}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{5,5}, {2,2}}));
      }
      {
        Rect r1 = {{-10,0}, {10,10}};
        Rect r2 = {{0,0}, {15,15}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{0,0}, {0,0}}));
      }
      {
        Rect r1 = {{-10,0}, {12,12}};
        Rect r2 = {{0,0}, {15,15}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{0,0}, {2,12}}));
      }
      {
        Rect r1 = {{10,10}, {150,150}};
        Rect r2 = {{0,0}, {300,200}};
        Rect re = r1.Extract(r2);
        assert(re == Rect({{10,10}, {150,150}}));
      }
      {
        Vec2 a(100, 100);
        assert(a.Normalize() == Vec2(1,1));
        Vec2 b(100, 99);
        assert(b.Normalize() == Vec2(1,1));
        Vec2 c(100, 50);
        assert(c.Normalize() == Vec2(1,1));
        Vec2 d(100, 49);
        assert(d.Normalize() == Vec2(1,0));
        Vec2 e(100, 1);
        assert(e.Normalize() == Vec2(1,0));
      }
      {
        Vec2 e(-100, 1);
        assert(e.Normalize() == Vec2(-1,0));
      }
      {
        Vec2 e(-100, -1);
        assert(e.Normalize() == Vec2(-1,0));
      }
      {
        Vec2 e(-100, 50);
        assert(e.Normalize() == Vec2(-1,1));
      }
    }

    void Viewport::CreateMap()
    {
      PartialMapsManager::RemoveMapArgs mapsToRemove;
      PartialMapsManager::CreateMapArgs newMaps;

      PerformMove(newMaps, mapsToRemove);

      m_mapManager.m_visibleArea = tt_loadedPixelRect;
      WorldModelDiffVect worldUpdateResult;
      m_mapManager.Update(newMaps, mapsToRemove, worldUpdateResult, 0);

      auto currentMaps = m_mapManager.GetMaps();
      for (const auto& m : currentMaps)
      {
        Vec2 pos = m.first - tt_loadedPixelRect.origin;
        m.second->Transfrorm(cocos2d::Vec2(pos.x, pos.y) * kSpritePosition,
                             1.f);
      }
    }

    void Viewport::Resize(const cocos2d::Size& size)
    {
      tt_viewSize = size;
      m_performMove = true;
    }

    void Viewport::UpdateAsync(float& updateTime)
    {
    }

    bool Viewport::IsAvailable()
    {
      return true;
    }
    
    bool Viewport::Destroy()
    {
      m_worldUpdateResult.clear();
      m_worldModel->Stop();
      return false;
    }

    cocos2d::Node* Viewport::GetRootNode() const
    {
      return m_superView;
    }

    cocos2d::Node* Viewport::GetMainNode() const
    {
      return m_mainView;
    }

    cocos2d::Node* Viewport::GetLightNode() const
    {
      return m_lightNode;
    }

    void Viewport::Update(float updateTime, float& outUpdateTime)
    {
      m_worldUpdateResult.clear();
      m_worldModel->PlayUpdates(config::numberOfUpdatesPerTick, tt_loadedPixelRect, m_worldUpdateResult);

      PartialMapsManager::RemoveMapArgs mapsToRemove;
      PartialMapsManager::CreateMapArgs newMaps;

      bool enableAnimations = m_mapManager.m_enableAnimations;
      bool enableFancyAnimaitons = m_mapManager.m_enableFancyAnimaitons;
      float greatPixelSize = m_superView->getScale() * kSpritePosition;
      if (greatPixelSize >= 7.f)
      {
        enableAnimations = true;
        enableFancyAnimaitons = true;
      }
      else if (greatPixelSize >= 3.f)
      {
        enableAnimations = true;
        enableFancyAnimaitons = false;
      }
      else
      {
        enableAnimations = false;
        enableFancyAnimaitons = false;
      }
      
      if (m_performMove)
      {
        PerformMove(newMaps, mapsToRemove);
      }

      m_mapManager.EnableAnimation(enableAnimations, enableFancyAnimaitons);
      m_mapManager.m_visibleArea = tt_loadedPixelRect;
      m_mapManager.Update(newMaps, mapsToRemove, m_worldUpdateResult, updateTime);

      if (m_performMove)
      {
        auto currentMaps = m_mapManager.GetMaps();
        for (const auto& m : currentMaps)
        {
          Vec2 pos = m.first - tt_loadedPixelRect.origin;
          m.second->Transfrorm(cocos2d::Vec2(pos.x, pos.y) * kSpritePosition, 1.f);
        }
        m_performMove = false;
      }
    }

    void Viewport::PerformMove(PartialMapsManager::CreateMapArgs& newMapsArgs,
                               PartialMapsManager::RemoveMapArgs& mapsToRemove)
    {
      auto graphicalVisibleRect = GetCurrentGraphicRect();

      Rect pixelRect = TTPixelRect(graphicalVisibleRect);
      Rect innerPrevRect = ResizeByStep(tt_loadedPixelRect, pixelRect, kSegmentSize);
      Rect extendedRect = ExtendRectWithStep(innerPrevRect, pixelRect, kSegmentSize);
      Vec2 size = m_worldModel->GetSize();
      extendedRect = extendedRect.Extract({Vec2(0, 0), size});
      
      if ( extendedRect == tt_loadedPixelRect || extendedRect.size == Vec2())
        return;

      Rect reusedRect = tt_loadedPixelRect.Extract(extendedRect);

      auto diffOfLoadedPixels = extendedRect.origin - tt_loadedPixelRect.origin;
      cocos2d::Vec2 currentSuperViewPos = m_superView->getPosition();

      cocos2d::Vec2 newSupperViewPos = currentSuperViewPos + FromPixels(diffOfLoadedPixels) * kSpritePosition * m_superView->getScale();
      m_superView->setPosition(newSupperViewPos);
      tt_loadedPixelRect = extendedRect;
      

      // remove maps out of visible rect
      auto currentMaps = m_mapManager.GetMaps();
      bool res = RemoveMapsOutsideOfRect(extendedRect, currentMaps, mapsToRemove);

      // get new maps to create
      std::vector<Rect> mapRects;
      res = SplitRectOnChunks(tt_loadedPixelRect, reusedRect, mapRects);
      assert(res);

      // fill output args
      res = FillCreateMapsArgs(mapRects,
                               newMapsArgs);
    }

    bool Viewport::RemoveMapsOutsideOfRect(const Rect& rect,
                                           const Maps& currentMaps,
                                           PartialMapsManager::RemoveMapArgs& mapsToRemove)
    {
      for (const auto& map : currentMaps)
      {
        Rect mapRect;
        mapRect.origin.x = map.second->m_a1;
        mapRect.origin.y = map.second->m_b1;
        mapRect.size.x = map.second->m_width;
        mapRect.size.y = map.second->m_height;

        Rect newMapRect = mapRect.Extract(rect);
        if (newMapRect.size.x == 0 || newMapRect.size.y == 0)
        {
          mapsToRemove.push_back(mapRect.origin);
        }
      }

      return true;
    }

    bool Viewport::SplitRectOnChunks(const Rect& rect, const Rect& existingRect, std::vector<Rect>& result) const
    {
      return jevo::SplitRectOnChunks(rect, existingRect, kSegmentSize, result);
    }

    bool Viewport::FillCreateMapsArgs(const std::vector<Rect>& rects,
                                      PartialMapsManager::CreateMapArgs& newMapsArgs)
    {
      for (auto rect : rects)
      {
        PartialMapsManager::CreateMapArg createMapArg;
        createMapArg.rect = rect;
        createMapArg.graphicPos = cocos2d::Vec2(rect.origin.x * kSpritePosition,
                                                rect.origin.y * kSpritePosition);
        newMapsArgs.push_back(createMapArg);
      }

      return true;
    }

    cocos2d::Rect Viewport::GetCurrentGraphicRect() const
    {
      cocos2d::Vec2 visibleGraphicalOrigin = FromPixels(tt_loadedPixelRect.origin) * kSpritePosition;
      visibleGraphicalOrigin = visibleGraphicalOrigin * m_superView->getScale();
      visibleGraphicalOrigin = visibleGraphicalOrigin - m_superView->getPosition();
      visibleGraphicalOrigin = visibleGraphicalOrigin / m_superView->getScale();

      cocos2d::Size visibleGraphicalSize = tt_viewSize / m_superView->getScale();

      cocos2d::Rect visibleRect (visibleGraphicalOrigin.x,
                                 visibleGraphicalOrigin.y,
                                 visibleGraphicalSize.width,
                                 visibleGraphicalSize.height);
      return visibleRect;
    }
  }
}



