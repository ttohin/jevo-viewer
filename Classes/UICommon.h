
#pragma once

#include "cocos2d.h"
#include "Common.h"

namespace jevo
{
  namespace graphic
  {
    cocos2d::Color3B ColorFromUint(uint32_t color);
    float RandomOffset();
    cocos2d::Vec2 RandomVectorOffset();
    cocos2d::Vec2 spriteVector(const jevo::Vec2& vec, const cocos2d::Vec2& vector = cocos2d::Vec2());
    cocos2d::Color3B randomColor(uint a = 0, uint b = 255);
    cocos2d::Vec2 FromPixels(jevo::Vec2ConstRef vec);
    jevo::Vec2 ToPixels(const cocos2d::Vec2& vec);
  }
}

