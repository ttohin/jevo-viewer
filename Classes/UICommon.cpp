
#include "UICommon.h"
#include "UIConfig.h"
#include "Random.h"

namespace jevo
{
  namespace graphic
  {
    cocos2d::Color3B ColorFromUint(uint32_t color)
    {
      uint8_t red = (color & 0x00FF0000) >> 16;
      uint8_t green = (color & 0x0000FF00) >> 8;
      uint8_t blue = (color & 0x000000FF) >> 0;
      
      return cocos2d::Color3B(red, green, blue);
    }
    
    float RandomOffset()
    {
      return cRandEps(0.0f, kTileFrameSize*0.1f);
    }
    
    cocos2d::Vec2 RandomVectorOffset()
    {
      return cocos2d::Vec2(RandomOffset(), RandomOffset());
    }
    
    cocos2d::Vec2 spriteVector(const jevo::Vec2& vec, const cocos2d::Vec2& vector)
    {
      return cocos2d::Vec2(vec.x * kSpritePosition, vec.y * kSpritePosition) + vector;
    }
    
    cocos2d::Color3B randomColor(uint bottomValue, uint topValue)
    {
      int r = cRandABInt(bottomValue, topValue);
      int g = cRandABInt(bottomValue, topValue);
      int b = cRandABInt(bottomValue, topValue);
      return cocos2d::Color3B(r, g, b);
    }
    
    cocos2d::Vec2 FromPixels(jevo::Vec2ConstRef vec)
    {
      return cocos2d::Vec2(vec.x, vec.y);
    }
    
    jevo::Vec2 ToPixels(const cocos2d::Vec2& vec)
    {
      return jevo::Vec2(vec.x, vec.y);
    }
  }
}
