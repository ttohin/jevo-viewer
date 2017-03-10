#include "AppDelegate.h"

#include <time.h>
#include <stdlib.h>
#include "LoadingScene.h"

#include "json.hpp"
#include "AsyncDiffReader.h"
#include <fstream>


USING_NS_CC;


AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
}

bool AppDelegate::applicationDidFinishLaunching()
{  
  timeval tval;
  gettimeofday(&tval, NULL);
  srand(tval.tv_usec);
  
  GLContextAttrs attr = {8, 8, 8, 8, 16, 0};
  GLView::setGLContextAttrs(attr);
  
  auto director = Director::getInstance();
  auto glview = director->getOpenGLView();
  if(!glview)
  {
    glview = GLViewImpl::createWithRect("Jevo 0.0.1", Rect(0, 0, 1024, 768), 1.0);
    
    director->setOpenGLView(glview);
    
    Size glViewSize = glview->getFrameSize();
    glview->setFrameSize(glViewSize.width, glViewSize.height);
  }
  
  /* director->setDisplayStats(true); */
  
  director->setAnimationInterval(1.0 / 60);
  auto scene = jevo::ui::LoadingScene::createScene();
  director->runWithScene(scene);
  
  return true;
}

void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();
}
