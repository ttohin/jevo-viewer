

#include <chrono>
#include <thread>
#include <iostream>
#include "LoadingScene.h"
#include "MainScene.h"
#include "Common.h"
#include "UIConfig.h"
#include "Logging.h"

#include "SharedUIData.h"

USING_NS_CC;

namespace jevo
{
  namespace ui
  {
    
    LoadingScene::~LoadingScene()
    {
      if (m_worldModel) m_worldModel->Stop();
    }
    
    bool LoadingScene::init()
    {
      if ( !Layer::init() )
      {
        return false;
      }
      
      m_viewport = nullptr;
      
      Size visibleSize = Director::getInstance()->getVisibleSize();
      auto origin = Director::getInstance()->getVisibleOrigin();
      
      m_info = CreateLabel("loading.", cocos2d::Vec2(12, visibleSize.height / 2));
      m_description1 = CreateLabel("", cocos2d::Vec2(12, visibleSize.height / 2 - 32 - 4));
      m_description2 = CreateLabel("", cocos2d::Vec2(12, visibleSize.height / 2 - 32 * 2 - 4));
      
      m_worldModel = std::make_shared<WorldModel>();
      
      schedule(schedule_selector(LoadingScene::LoadViewModel), 2, CC_REPEAT_FOREVER, 0);
      
      return true;
    }
    
    void LoadingScene::timerForUpdate(float dt)
    {
    }
    
    void LoadingScene::LoadViewModel(float dt)
    {
      if (m_worldModel->Init(jevo::config::workingFolder))
      {
        unschedule(schedule_selector(LoadingScene::LoadViewModel));
        schedule(schedule_selector(LoadingScene::CreateViewport), 0, 0, 0);
      }
      else
      {
        m_description1->setString("waiting for keyframe.json");
        m_description2->setString(config::workingFolder.c_str());
      }
    }
    
    void LoadingScene::CreateViewport(float dt)
    {
      auto rootNode = cocos2d::Node::create();
      rootNode->retain();
      Size visibleSize = Director::getInstance()->getVisibleSize();
      m_viewport = std::make_shared<jevo::graphic::Viewport>(rootNode, visibleSize, m_worldModel);
      m_worldModel = nullptr;
      
      auto mapScene = MainScene::createScene(m_viewport);
      Director::getInstance()->replaceScene(mapScene);
    }
    
    cocos2d::LabelProtocol* LoadingScene::CreateLabel(const char* text, const cocos2d::Vec2& offset)
    {
      auto result = LabelAtlas::create(text, "font1.png", 32, 32, 0);
      result->setPosition(offset);
      result->setScale(0.5, 0.8);
      addChild(result);
      return result;
    }
  }
}
