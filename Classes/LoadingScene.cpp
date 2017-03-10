

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
      
      m_info = CreateLabel("Loading", cocos2d::Vec2(visibleSize.width / 2, visibleSize.height / 2));
      
      schedule(schedule_selector(LoadingScene::LoadViewModel), 0, 0, 0);
      
      return true;
    }
    
    void LoadingScene::timerForUpdate(float dt)
    {
    }
    
    void LoadingScene::LoadViewModel(float dt)
    {
      m_worldModel = std::make_shared<WorldModel>();
      if (!m_worldModel->Init(jevo::config::workingFolder))
      {
        std::cout << FRED("Could not init world model") << std::endl;
        exit(-1);
      }
      
      schedule(schedule_selector(LoadingScene::CreateViewport), 0, 0, 0);
    }
    
    void LoadingScene::CreateViewport(float dt)
    {
      auto rootNode = cocos2d::Node::create();
      rootNode->retain();
      Size visibleSize = Director::getInstance()->getVisibleSize();
      m_viewport = std::make_shared<jevo::graphic::Viewport>(rootNode, visibleSize, m_worldModel);
      
      auto mapScene = MainScene::createScene(m_viewport);
      Director::getInstance()->replaceScene(mapScene);
    }
    
    cocos2d::LabelProtocol* LoadingScene::CreateLabel(const char* text, const cocos2d::Vec2& offset)
    {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
      auto result = LabelAtlas::create(text, "font.png", 18, 24, 32);
      result->setPosition(offset);
      addChild(result);
      return result;
#else
      auto result = Label::createWithSystemFont(text, "Menlo", 24, Size::ZERO, TextHAlignment::RIGHT);
      //  auto result = Label::createWithCharMap("font.png", 18, 24, 32);
      result->setString(text);
      result->setPosition(offset);
      result->setHorizontalAlignment(TextHAlignment::LEFT);
      addChild(result);
      return result;
#endif
    }
  }
}
