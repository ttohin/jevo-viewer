//
//  CellContext.cpp
//  Komorki
//
//  Created by user on 02.03.17.
//
//

#include "ObjectContext.h"
#include "Logging.h"
#include "UICommon.h"
#include "PartialMap.h"
#include "UIConfig.h"
#include "SpriteBatch.h"


namespace
{
  static const auto kSmallAnimationsTag = 99;
}

namespace jevo
{
  namespace graphic
  {
    
    
    uint ObjectContext::instanceCounter = 0;
    
    ObjectContext::ObjectContext(PartialMapPtr _owner,
                             const cocos2d::Color3B& color,
                             const cocos2d::Rect& textureRect,
                             Vec2ConstRef origin,
                             const Rect& rect)
    {
      instanceCounter += 1;
      m_owner = _owner;
      m_posOffset = rect.origin - origin;
      m_offset = cocos2d::Vec2(0, 0); //= RandomVectorOffset();
      m_size = rect.size;
      m_pos = GetPosInOwnerBase(origin);
      m_textureRect = textureRect;
      
      cocos2d::Vec2 rectOffset = spriteVector(m_size) * 0.5;
      
      auto s = m_owner->m_cellMap->CreateSprite();
      s->setTextureRect(textureRect);
      s->setScale(kSpriteScale);
      s->setAnchorPoint({0.5, 0.5});
      s->setColor(color);
      
      s->setPosition(spriteVector(m_pos + m_posOffset, m_offset + rectOffset));
      s->setTag(1);
      m_sprite = s;
    }
    
    ObjectContext::~ObjectContext()
    {
      instanceCounter -= 1;
    }
    
    void ObjectContext::Move(Vec2ConstRef src, Vec2ConstRef dest, float animationTime)
    {
      Vec2 localSrc = GetPosInOwnerBase(src);
      Vec2 localDest = GetPosInOwnerBase(dest);
      
      auto randOffset = cocos2d::Vec2(0, 0);
      cocos2d::Vec2 offset = m_offset;
      m_offset = randOffset;
      cocos2d::Vec2 rectOffset = spriteVector(m_size) * 0.5;
      
      if (m_owner->m_enableAnimations)
      {
        m_sprite->stopAllActionsByTag(0);
        m_sprite->setPosition(spriteVector(localSrc + m_posOffset, offset + rectOffset));
        auto moveTo = cocos2d::MoveTo::create(animationTime, spriteVector(localDest + m_posOffset, randOffset + rectOffset));
        moveTo->setTag(0);
        m_sprite->runAction(moveTo);
      }
      else
      {
        m_sprite->setPosition(spriteVector(localDest + m_posOffset, randOffset + rectOffset));
      }
      
      m_pos = GetPosInOwnerBase(dest);
    }

    
    void ObjectContext::BecomeOwner(PartialMapPtr _owner)
    {
      m_sprite->retain();
      m_sprite->removeFromParentAndCleanup(true);
      _owner->m_cellMap->addChild(m_sprite);
      m_sprite->release();
      m_sprite->setPosition(spriteVector(m_pos, m_offset));
      
      m_owner = _owner;
    }
    
    void ObjectContext::Destory(PartialMapPtr _owner)
    {
      assert(m_sprite);
      m_owner->m_cellMap->RemoveSprite(m_sprite);
      m_owner = nullptr;
    }
    
    void ObjectContext::Attack(const Vec2& pos, const Vec2& attackOffset, float animationDuration)
    {
      Vec2 localSrc = GetPosInOwnerBase(pos);
      
      cocos2d::Vec2 offset = m_offset;
      cocos2d::Vec2 rectOffset = spriteVector(m_size) * 0.5;
      
      if (m_owner->m_enableAnimations)
      {
        m_sprite->stopAllActionsByTag(10);
        m_sprite->setPosition(spriteVector(localSrc + m_posOffset, offset + rectOffset));
        auto m1 = cocos2d::MoveTo::create(animationDuration * 0.3, 0.5 * spriteVector(attackOffset) + spriteVector(localSrc + m_posOffset, offset + rectOffset));
        auto m2 = cocos2d::MoveTo::create(animationDuration * 0.3, spriteVector(localSrc + m_posOffset, offset + rectOffset));
        auto s1 = cocos2d::ScaleTo::create(animationDuration*0.3, kSpriteScale * 1.2, kSpriteScale * 1.2);
        auto s2 = cocos2d::ScaleTo::create(animationDuration*0.3, kSpriteScale, kSpriteScale);
        auto spawn1 = cocos2d::Spawn::createWithTwoActions(m1, s1);
        auto spawn2 = cocos2d::Spawn::createWithTwoActions(m2, s2);
        auto seq = cocos2d::Sequence::createWithTwoActions(spawn1, spawn2);
        seq->setTag(10);
        m_sprite->runAction(seq);
      }
    }
    
    void ObjectContext::ToggleAnimation()
    {
      return;
      
      if (!m_owner->m_enableFancyAnimations)
      {
        m_sprite->stopAllActionsByTag(kSmallAnimationsTag);
        return;
      }
      
      m_sprite->stopAllActionsByTag(kSmallAnimationsTag);
      bool scaleDirection = rand()%2;
      auto s1 = cocos2d::ScaleTo::create(2, kSpriteScale * 0.8, kSpriteScale * 0.8);
      auto s2 = cocos2d::ScaleTo::create(2, kSpriteScale * 1.1, kSpriteScale * 1.1);
      cocos2d::ActionInterval* loop = nullptr;
      if (scaleDirection)
        loop = cocos2d::RepeatForever::create(cocos2d::Sequence::create(s1, s2, NULL));
      else
        loop = cocos2d::RepeatForever::create(cocos2d::Sequence::create(s2, s1, NULL));;
      loop->setTag(kSmallAnimationsTag);
      m_sprite->runAction(loop);
    }
    
    void ObjectContext::CellDead()
    {
      auto s = m_owner->m_background->CreateSprite();
      
      s->setTextureRect(m_textureRect);
      s->setPosition(m_sprite->getPosition());
      s->setOpacity(130);
      s->setScale(m_sprite->getScale());

      auto fade = cocos2d::FadeTo::create(5, 0);
      auto bgLayer = m_owner->m_background;
      auto removeSelf = cocos2d::CallFunc::create([bgLayer, s]()
                                         {
                                           bgLayer->RemoveSprite(s);
                                         });
      
      s->runAction(cocos2d::Sequence::createWithTwoActions(fade, removeSelf));
    }
    
    std::string ObjectContext::Description() const
    {
      std::stringstream ss;
      ss <<
      "[" <<
      static_cast<const void*>(this) <<
      "]";
      return ss.str();
    }
    
    
    Vec2 ObjectContext::GetPosInOwnerBase(Vec2ConstRef pos) const
    {
      return pos - Vec2(m_owner->m_a1, m_owner->m_b1);
    }
  }
}
