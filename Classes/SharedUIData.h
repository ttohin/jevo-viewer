#pragma once

#include <list>
#include <unordered_map>
#include <cocos2d.h>


namespace jevo
{
  namespace graphic
  {
    class SharedUIData
    {
    public:
      static SharedUIData* getInstance();
      
      typedef std::unordered_map<uint, cocos2d::Rect> CellShapeTexturesMap;

      
      CellShapeTexturesMap m_textureMap;
    };
  }
}

