
#pragma once

#include "Viewport.h"

namespace jevo
{
  class WorldModel;
  
  namespace ui
  {
    class LoadingScene: public cocos2d::Layer
    {
    public:
      virtual bool init();
      virtual ~LoadingScene();
      static cocos2d::Scene* createScene()
      {
        auto scene = cocos2d::Scene::create();

        LoadingScene *pRet = new(std::nothrow) LoadingScene();
        if (pRet && pRet->init())
        {
          pRet->autorelease();
          scene->addChild(pRet);
          return scene;
        }
        else
        {
          delete pRet;
          pRet = NULL;
          return NULL;
        }
      }

  cocos2d::LabelProtocol* CreateLabel(const char* text, const cocos2d::Vec2& offset);
  void timerForUpdate(float dt);
  void LoadViewModel(float dt);
  void CreateViewport(float dt);

private:
  cocos2d::LabelProtocol* m_info;
  std::shared_ptr<WorldModel> m_worldModel;
  jevo::graphic::Viewport::Ptr m_viewport;
  std::vector<std::string> m_mapList;

};

}
}

