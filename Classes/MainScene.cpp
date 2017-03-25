#include "MainScene.h"
#include "WorldModel.h"
#include "cocostudio/DictionaryHelper.h"
#include "cocos2d/cocos/ui/CocosGUI.h"
#include "LoadingScene.h"
#include "UIConfig.h"

static const float kZoomStep = 0.05;

static float kButtonScale = 1.f;
static float kButtonSize = 40.f;
static float kViewportUpdateTime = 0.1;

USING_NS_CC;
using namespace cocostudio;

void MainScene::CreateMap(const jevo::graphic::Viewport::Ptr& viewport)
{
  m_viewport = viewport;
  addChild(m_viewport->GetRootNode());
}

bool MainScene::init(const jevo::graphic::Viewport::Ptr& viewport)
{
  if ( !Layer::init() )
  {
    return false;
  }

  auto glview = Director::getInstance()->getOpenGLView();
  bool isRetina = glview->isRetinaDisplay();
  if (isRetina)
  {
    kButtonScale *= 2.f;
    kButtonSize *= 2.f;
  }

  m_stopManager = false;
  m_updateTime = jevo::config::updateTime;
  m_pause = false;

  Size visibleSize = Director::getInstance()->getVisibleSize();

  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  Size winSize = Director::getInstance()->getWinSize();

  m_bg = Sprite::create("mapBackground.png");
  m_bg->setAnchorPoint({0, 0});
  addChild(m_bg, -999);

  float bgAspect = 1.0;
  {
    bgAspect = AspectToFill(m_bg->getContentSize(), visibleSize);
  }
  m_bg->setScale(bgAspect);

  m_rootNode = Node::create();
  addChild(m_rootNode);
  m_menuNode = Node::create();
  addChild(m_menuNode);

  CreateMap(viewport);

  schedule(schedule_selector(MainScene::timerForUpdate), m_updateTime, kRepeatForever, m_updateTime);
  schedule(schedule_selector(MainScene::timerForViewportUpdate), kViewportUpdateTime, kRepeatForever, kViewportUpdateTime);

#if CC_TARGET_PLATFORM != CC_PLATFORM_IOS
  auto touchListener = EventListenerTouchOneByOne::create();
  touchListener->setSwallowTouches(true);

  touchListener->onTouchMoved = [this](Touch* touch, Event* event)
  {
    this->Move(touch->getDelta());
  };

  touchListener->onTouchBegan = [this](Touch* touch, Event* event)
  {
    return true;
  };

  touchListener->onTouchEnded = [this](Touch* touch, Event* event)
  {
  };

#else // CC_TARGET_PLATFORM == CC_PLATFORM_IOS

  auto touchListener = EventListenerTouchAllAtOnce::create();
  touchListener->onTouchesMoved = [this](const std::vector<Touch*>& touches, Event* event)
  {
    if (touches.size() == 1)
    {
      m_mapPos = m_rootNode->getPosition() + touches[0]->getDelta();
    }
    else if (touches.size() == 2)
    {
      Vec2 delta = (touches[0]->getDelta() + touches[1]->getDelta()) * 0.5;
      this->Move(delta);

      float initialDistance = touches[0]->getPreviousLocation().distance(touches[1]->getPreviousLocation());
      float currentDistance = touches[0]->getLocation().distance(touches[1]->getLocation());

      float scale = 0.5 * (currentDistance - initialDistance)/initialDistance;
      this->Zoom(scale);
    }
  };

#endif // CC_TARGET_PLATFORM != CC_PLATFORM_IOS

  _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

#if CC_TARGET_PLATFORM != CC_PLATFORM_IOS
  auto mouseListener = EventListenerMouse::create();
  mouseListener->onMouseScroll = [this](Event* event)
  {
    if(this->m_currenMenu)
    {
      m_currenMenu->OnMouseScroll(event);
      return;
    }


    EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
    float scrollY = mouseEvent->getScrollY();

    if (scrollY > 0)
      scrollY = 0.02;
    else
      scrollY = -0.02;

    float cursorX = mouseEvent->getCursorX();
    float cursorY = mouseEvent->getCursorY();

    if (m_viewport) m_viewport->Zoom({cursorX, cursorY}, scrollY);
  };

  mouseListener->onMouseMove = [this](Event* event) {
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

  auto keyboardListener = EventListenerKeyboard::create();
  keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event)
  {
    if(this->m_currenMenu)
    {
      m_currenMenu->OnKeyRelease(keyCode, event);
      return;
    }

    if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
    {
      this->ShowMainMenu();
      return;
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_Q)
    {
      this->Exit();
    }
  };

  keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event)
  {
    if(this->m_currenMenu)
    {
      m_currenMenu->OnKeyPressed(keyCode, event);
      return;
    }
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
  
  auto menuButton = ui::Button::create("menuButton.png", "menuButton_hl.png");
  menuButton->setTitleText("Meny");
  menuButton->addTouchEventListener([this](Ref*,ui::Widget::TouchEventType controlEvent)
                                      {
                                        if (controlEvent == ui::Widget::TouchEventType::ENDED)
                                          this->ShowMainMenu();
                                      });
  this->addChild(menuButton);
  menuButton->setPosition(Vec2(kButtonSize/2.f, visibleSize.height - kButtonSize/2.f));
  menuButton->setScale(kButtonScale);
  m_menuButton = menuButton;
  m_menuButton->setVisible(false);

  CreateSpeedToolBar();
  m_speedToolbar->setPosition(Vec2(origin.x + visibleSize.width,
                                   origin.y));
  m_speedToolbar->setVisible(false);

  return true;
}

void MainScene::ShowMainMenu()
{
}


void MainScene::SetCurrentMenu(const std::shared_ptr<IFullScreenMenu> menu)
{
  if (m_currenMenu == menu)
  {
    return;
  }

  if (m_currenMenu)
  {
    m_currenMenu->Hide();
  }

  m_currenMenu = menu;

  m_currenMenu->ShowInView(m_menuNode);
  m_pause = true;
}

void MainScene::RestartMap()
{
  m_stopManager = true;
  m_pause = false;
  m_currenMenu->Hide();
  m_currenMenu = nullptr;
}

void MainScene::Exit()
{
  exit(0);
}

void MainScene::ShowMainScreen()
{
  m_pause = false;
  m_currenMenu->Hide();
  m_currenMenu = nullptr;
}

void MainScene::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags)
{
  Size viewSize = Director::getInstance()->getVisibleSize();
  Size contentSize = getContentSize();

  if (!viewSize.equals(contentSize))
  {
    // before we visit we are going to set our size to the visible size.
    setContentSize(Director::getInstance()->getVisibleSize());

    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Size visibleSize = Director::getInstance()->getVisibleSize();
    m_speedToolbar->setPosition(Vec2(origin.x + visibleSize.width,
                                     origin.y));
    m_menuButton->setPosition(Vec2(kButtonSize/2.f, visibleSize.height - kButtonSize/2.f));
    if (m_currenMenu) m_currenMenu->Resize(visibleSize);

    if (m_viewport) m_viewport->Resize(visibleSize);

    float bgAspect = 1.0;
    {
      bgAspect = AspectToFill(m_bg->getContentSize(), visibleSize);
    }
    m_bg->setScale(bgAspect);
  }

  Layer::visit(renderer, parentTransform, parentFlags);
}

void MainScene::timerForUpdate(float dt)
{
  if (m_pause || m_speed == eSpeedPause) return;

  if(m_viewport && !m_viewport->IsAvailable()) return;

  if (m_stopManager)
  {
    if (m_viewport->Destroy())
    {
      m_viewport = nullptr;
      m_stopManager = false;
      
      auto loadingScene = jevo::ui::LoadingScene::createScene();
      Director::getInstance()->replaceScene(loadingScene);
    }
    return;
  }

  if (m_speed == eSpeedNormal || m_speed == eSpeedDouble)
  {
    float updateTime = m_speed == eSpeedNormal ? m_updateTime : m_updateTime * 0.2;
    float updateTimeEstimated = updateTime;
    float tmp;
    if (m_viewport) m_viewport->Update(m_updateTime, tmp);

    if (updateTimeEstimated > updateTime)
    {
      updateTimeEstimated = 0.f;
    }
    schedule(schedule_selector(MainScene::timerForUpdate), updateTimeEstimated, kRepeatForever, updateTimeEstimated);
  }
  else if (m_speed == eSpeedMax)
  {
    float updateTimeEstimated = m_updateTime;
    if (m_viewport) m_viewport->UpdateAsync(updateTimeEstimated);
    schedule(schedule_selector(MainScene::timerForUpdate), 0, kRepeatForever, 0);
  }

  m_prevSpeed = m_speed;
}

void MainScene::timerForViewportUpdate(float dt)
{
  if (m_viewport) m_viewport->Calculate();
}

void MainScene::Zoom(float direction)
{
  Size visibleSize = Director::getInstance()->getVisibleSize();
  float cursorX = visibleSize.width/2;
  float cursorY = visibleSize.height/2;

  if (m_viewport) m_viewport->Zoom({cursorX, cursorY}, direction);
}

void MainScene::ZoomIn()
{
  Zoom(-kZoomStep);
}

void MainScene::ZoomOut()
{
  Zoom(+kZoomStep);
}

void MainScene::Move(const Vec2& direction, float animationDuration)
{
  if (m_viewport) m_viewport->Move(direction);
}

void MainScene::CreateSpeedToolBar()
{
  m_speedToolbar = Node::create();
  addChild(m_speedToolbar, 999);

  auto speed1Button = ui::Button::create("speed1.png", "speed1_hl.png");
  speed1Button->addTouchEventListener([this](Ref*,ui::Widget::TouchEventType controlEvent)
                                        {
                                          if (controlEvent == ui::Widget::TouchEventType::ENDED)
                                            this->SetSpeed(eSpeedNormal);
                                        });
  m_speedToolbar->addChild(speed1Button);
  speed1Button->setPosition(Vec2(-kButtonSize/2.f - kButtonSize*2, +kButtonSize/2.f));
  speed1Button->setScale(kButtonScale);
  m_speed1Button = speed1Button;

  auto speed2Button = ui::Button::create("speed2.png", "speed2_hl.png");
  speed2Button->addTouchEventListener([this](Ref*,ui::Widget::TouchEventType controlEvent)
                                      {
                                        if (controlEvent == ui::Widget::TouchEventType::ENDED)
                                          this->SetSpeed(eSpeedDouble);
                                      });
  m_speedToolbar->addChild(speed2Button);
  speed2Button->setPosition(Vec2(-kButtonSize/2.f - kButtonSize*1, +kButtonSize/2.f));
  speed2Button->setScale(kButtonScale);
  m_speed2Button = speed2Button;

  auto speedWarpButton = ui::Button::create("speed10.png", "speed10_hl.png");
  speedWarpButton->addTouchEventListener([this](Ref*,ui::Widget::TouchEventType controlEvent)
                                      {
                                        if (controlEvent == ui::Widget::TouchEventType::ENDED)
                                          this->SetSpeed(eSpeedMax);
                                      });
  m_speedToolbar->addChild(speedWarpButton);
  speedWarpButton->setPosition(Vec2(-kButtonSize/2.f - kButtonSize*0, +kButtonSize/2.f));
  speedWarpButton->setScale(kButtonScale);
  m_speed10Button = speedWarpButton;

  {
    auto button = ui::Button::create("pauseBtn.png.png", "pauseBtn.png");
    button->addTouchEventListener([this](Ref*,ui::Widget::TouchEventType controlEvent)
                                  {
                                    if (controlEvent == ui::Widget::TouchEventType::ENDED)
                                      this->SetSpeed(eSpeedPause);
                                  });
    m_speedToolbar->addChild(button);
    button->setPosition(Vec2(-kButtonSize/2.f - kButtonSize*3, +kButtonSize/2.f));
    button->setScale(kButtonScale);
    m_pauseButton = button;
  }

//  SetSpeed(eSpeedPause);
  SetSpeed(eSpeedNormal);
}

void MainScene::SetSpeed(Speed speed)
{
  if (m_speed == eSpeedNormal)
    m_speed1Button->loadTextureNormal("speed1.png");
  if (m_speed == eSpeedDouble)
    m_speed2Button->loadTextureNormal("speed2.png");
  if (m_speed == eSpeedMax)
    m_speed10Button->loadTextureNormal("speed10.png");
  if (m_speed == eSpeedPause)
    m_pauseButton->loadTextureNormal("pauseBtn.png");

  m_speed = speed;

  if (m_speed == eSpeedNormal)
    m_speed1Button->loadTextureNormal("speed1_sel.png");
  if (m_speed == eSpeedDouble)
    m_speed2Button->loadTextureNormal("speed2_sel.png");
  if (m_speed == eSpeedMax)
    m_speed10Button->loadTextureNormal("speed10_sel.png");
  if (m_speed == eSpeedPause)
    m_pauseButton->loadTextureNormal("pauseBtn_sel.png");
}

float MainScene::AspectToFit(const Size& source, const Size& target)
{
  float wAspect = target.width/source.width;
  float hAspect = target.height/source.height;
  return MIN(wAspect, hAspect);
}

float MainScene::AspectToFill(const Size& source, const Size& target)
{
  float wAspect = target.width/source.width;
  float hAspect = target.height/source.height;
  return MAX(wAspect, hAspect);
}

