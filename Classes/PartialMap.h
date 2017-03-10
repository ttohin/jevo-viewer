
#pragma once

#include <memory>
#include <unordered_map>
#include <list>
#include "Common.h"
#include "cocos2d.h"

namespace jevo
{
  class IPixelWorld;
  
  namespace graphic
  {
    class SpriteBatch;
    
    class PartialMap
    {
    public:
      
      using Ptr = std::shared_ptr<PartialMap>;
      
      static std::uint32_t instanceCounter;
      
      PartialMap();
      virtual ~PartialMap();
      
      bool Init(int a,
                int b,
                int width,
                int height,
                cocos2d::Node* superView,
                cocos2d::Node* lightNode,
                const cocos2d::Vec2& offset);

      void EnableFancyAnimations(bool enable);
      void EnableAnimations(bool enable);
      
      void Transfrorm(const cocos2d::Vec2& pos, float scale);
      void ChangeAABB(int a, int b, int width, int height);
      
      std::string Description();
      
      int m_a1;
      int m_b1;
      int m_width;
      int m_height;
      int m_a2;
      int m_b2;
      SpriteBatch* m_cellMap;
      SpriteBatch* m_background;
      
      // just holders for ObjectContexts
      bool m_enableAnimations = false;
      bool m_enableFancyAnimations = false;
      
    private:

      SpriteBatch* m_terrainBgSprite;
    };
  }
}

