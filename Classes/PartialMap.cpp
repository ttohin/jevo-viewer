

#include "PartialMap.h"
#include "Utilities.h"
#include "UIConfig.h"
#include <stdio.h>
#include <sstream> //for std::stringstream
#include <string>  //for std::string
#include "UICommon.h"
#include "Logging.h"
#include "SpriteBatch.h"

namespace jevo
{
  namespace graphic
  {
    uint PartialMap::instanceCounter = 0;
    
    PartialMap::PartialMap()
    {
      instanceCounter += 1;
      
      
    }
    
    PartialMap::~PartialMap()
    {
      instanceCounter -= 1;

      m_cellMap->removeAllChildren();
      m_cellMap->removeFromParentAndCleanup(true);
      m_background->removeAllChildren();
      m_background->removeFromParentAndCleanup(true);
      m_terrainBgSprite->removeFromParentAndCleanup(true);
    }
    
    bool PartialMap::Init(int a,
                          int b,
                          int width,
                          int height,
                          cocos2d::Node* superView,
                          cocos2d::Node* lightNode,
                          const cocos2d::Vec2& offset)
    {
      ChangeAABB(a, b, width, height);
      m_width = width;
      m_height = height;
      
      m_cellMap = new SpriteBatch(); m_cellMap->autorelease();
      m_background = new SpriteBatch();  m_background->autorelease();
      m_terrainBgSprite = new SpriteBatch();  m_terrainBgSprite->autorelease();

      
      m_cellMap->setName("m_cellMap " + Description());
      m_background->setName("m_background " + Description());
      m_terrainBgSprite->setName("m_terrainBgSprite " + Description());
      
      std::string cellTextureFileName = "blank.png";
      m_cellMap->init(cellTextureFileName);
      m_background->init(cellTextureFileName);
      m_terrainBgSprite->init(cellTextureFileName);
      
      auto bgSprite = m_terrainBgSprite->CreateSprite();
      bgSprite->setAnchorPoint({0, 0});
      bgSprite->setPosition({0, 0});
      bgSprite->setScale(kSpritePosition * kSegmentSize);
      
      m_cellMap->setPosition(offset);
      m_background->setPosition(offset);
      m_terrainBgSprite->setPosition(offset);
      
      superView->addChild(m_terrainBgSprite, -1);
      superView->addChild(m_background, 0);
      superView->addChild(m_cellMap, 1);
      
      if (kRandomColorPerPartialMap)
        bgSprite->setColor(randomColor(230, 255));
      
      return true;
    }
    
    void PartialMap::EnableFancyAnimations(bool enable)
    {
      m_enableFancyAnimations = enable;
    }
    
    void PartialMap::EnableAnimations(bool enable)
    {
      m_enableAnimations = enable;
    }
    
    void PartialMap::Transfrorm(const cocos2d::Vec2& pos, float scale)
    {
      m_cellMap->setPosition(pos);
      m_background->setPosition(pos);
      m_terrainBgSprite->setPosition(pos);
      
      m_cellMap->setScale(scale);
      m_background->setScale(scale);
      m_terrainBgSprite->setScale(scale);
    }
    
    void PartialMap::ChangeAABB(int a, int b, int width, int height)
    {
      m_a1 = a;
      m_a2 = a + width;
      m_b1 = b;
      m_b2 = b + height;
    }
    
    std::string PartialMap::Description()
    {
      char buf[1024];
      snprintf(buf, 1024 - 1, "%p x:[%d,%d] y:[%d,%d] %p", this, m_a1, m_a2, m_b1, m_b2, m_cellMap);
      return std::string(buf);
    }
  }
}
