
#include "GraphicContext.h"
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
    std::uint32_t GraphicContext::instanceCounter = 0;
    
    GraphicContext::GraphicContext(uint64_t id,
                                   PartialMapPtr _owner,
                                   const cocos2d::Color3B& color,
                                   const cocos2d::Rect& textureRect,
                                   Vec2ConstRef origin,
                                   const Rect& rect)
    {
      m_id = id;
      instanceCounter += 1;
      m_owner = _owner;
      
      auto it = m_owner->m_contexts.insert(this);
      assert(it.second);
      
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
      
      LOG_W("%s %s", __FUNCTION__, Description().c_str());
    }
    
    GraphicContext::~GraphicContext()
    {
      LOG_W("%s %s", __FUNCTION__, Description().c_str());
      instanceCounter -= 1;
      Destory();
    }
    
    void GraphicContext::Move(Vec2ConstRef src, Vec2ConstRef dest, float animationTime)
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
    
    
    void GraphicContext::BecomeOwner(PartialMapPtr _owner)
    {
      LOG_W("%s %s", __FUNCTION__, Description().c_str());
      
      if (m_owner != _owner)
      {
        auto it = _owner->m_contexts.insert(this);
        assert(it.second);
        m_owner->m_contexts.erase(this);
      }
      
      m_sprite->retain();
      m_sprite->removeFromParentAndCleanup(true);
      _owner->m_cellMap->addChild(m_sprite);
      m_sprite->release();
      m_sprite->setPosition(spriteVector(m_pos, m_offset));
      
      m_owner = _owner;
    }
    
    void GraphicContext::Destory()
    {
      LOG_W("%s %s", __FUNCTION__, Description().c_str());
      
      if (!m_owner)
        return;
      
      m_owner->m_contexts.erase(this);
      
      assert(m_sprite);
      m_owner->m_cellMap->RemoveSprite(m_sprite);
      m_owner = nullptr;
    }
    
    void GraphicContext::Attack(const Vec2& pos, const Vec2& attackOffset, float animationDuration)
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
    
    void GraphicContext::ToggleAnimation()
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
    
    void GraphicContext::Alert(cocos2d::Color3B alertColor)
    {
      auto s = m_owner->m_background->CreateSprite();
      
      s->setTextureRect(m_textureRect);
      s->setPosition(m_sprite->getPosition());
      s->setOpacity(130);
      s->setScale(m_sprite->getScale() * 2);
      s->setColor(alertColor);
      
      auto fade = cocos2d::FadeTo::create(5, 0);
      auto bgLayer = m_owner->m_background;
      auto removeSelf = cocos2d::CallFunc::create([bgLayer, s]()
                                                  {
                                                    bgLayer->RemoveSprite(s);
                                                  });
      
      s->runAction(cocos2d::Sequence::createWithTwoActions(fade, removeSelf));
    }
    
    void GraphicContext::FadeCell()
    {
      auto s = m_owner->m_background->CreateSprite();
      
      s->setTextureRect(m_textureRect);
      s->setPosition(m_sprite->getPosition());
      s->setOpacity(config::fadeInitialOpacity);
      s->setScale(m_sprite->getScale());
      s->setColor(m_sprite->getColor());
      
      auto fade = cocos2d::FadeTo::create(config::fadeDuration, 0);
      auto bgLayer = m_owner->m_background;
      auto removeSelf = cocos2d::CallFunc::create([bgLayer, s]()
                                                  {
                                                    bgLayer->RemoveSprite(s);
                                                  });
      
      s->runAction(cocos2d::Sequence::createWithTwoActions(fade, removeSelf));
    }
    
    std::string GraphicContext::Description() const
    {
      std::stringstream ss;
      ss <<
      "[" <<
      static_cast<const void*>(this) <<
      " owner: " << static_cast<const void*>(m_owner ? m_owner.get() : NULL) <<
      " id: " << m_id <<
      "]";
      return ss.str();
    }
    
    
    Vec2 GraphicContext::GetPosInOwnerBase(Vec2ConstRef pos) const
    {
      return pos - Vec2(m_owner->m_a1, m_owner->m_b1);
    }
  }
}
