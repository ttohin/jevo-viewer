
#pragma once

#include "cocos2d.h"
#include "Common.h"

namespace jevo
{
  namespace graphic
  {
    extern const float kTileFrameSize;
    extern const int kSpritePosition;
    extern const double kSpriteScale;
    extern const float kViewportMargin;
    extern const int kSegmentSize;
    extern const bool kAnimated;
    extern const bool kRedrawEachUpdate;
    extern const bool kSimpleDraw;
    extern const bool kRandomColorPerPartialMap;
    extern const float kLightMapScale;
    extern const unsigned int kNumberOfUpdatesOnStartup;
    extern const jevo::Vec2 kCellsTextureSizeInPixels;
    extern const unsigned int kCellShapeSegments;
    extern const unsigned int kSpriteBatchNodePullSize;
  }
  
  namespace config
  {
    const std::string workingFolder = cocos2d::FileUtils::getInstance()->getWritablePath() + "jevo";
    const float initialScale = 0.1;
    const unsigned int numberOfUpdatesPerTick = 1;
    const float updateTime = 0.0001;
    const bool healthCheck = false;
    const bool removeFiles = false;
  }
}

