//
//  CellContext.hpp
//  Komorki
//
//  Created by user on 02.03.17.
//
//

#pragma once

#include "Common.h"

namespace jevo
{
  namespace graphic
  {
  
    class PartialMap;
    
    class ObjectContext
    {
    public:
      
      static uint instanceCounter;
      
      using PartialMapPtr = std::shared_ptr<PartialMap>;
      
      ObjectContext(PartialMapPtr _owner,
                  const cocos2d::Color3B& color,
                  const cocos2d::Rect& textureRect,
                  Vec2ConstRef origin,
                  const Rect& rect);
      virtual ~ObjectContext();
      
      void Move(Vec2ConstRef src, Vec2ConstRef dest, float animationDuration);
      
      virtual void BecomeOwner(PartialMapPtr _owner) ;
      virtual void Destory(PartialMapPtr _owner) ;
      virtual void Attack(const Vec2& pos, const Vec2& offset, float animationTime) ;
      virtual void ToggleAnimation();
      virtual void CellDead();
      Vec2 GetPosInOwnerBase(Vec2ConstRef pos) const;
      std::string Description() const;
      
      
      cocos2d::Sprite* m_sprite = nullptr;
      cocos2d::Sprite* m_glow = nullptr;
      cocos2d::Vec2 m_offset;
      Vec2 m_pos;
      Vec2 m_posOffset; // for rect shapes
      Vec2 m_size; // for rect shapes
      cocos2d::Rect m_textureRect;
      PartialMapPtr m_owner;
      uint32_t m_updateId = 0;
    };
  }
}


