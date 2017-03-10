

#include "SharedUIData.h"

namespace jevo
{
  namespace graphic
  {
    // singleton stuff
    static SharedUIData *s_SharedDirector = nullptr;
    
    SharedUIData* SharedUIData::getInstance()
    {
      if (!s_SharedDirector)
      {
        s_SharedDirector = new (std::nothrow) SharedUIData();
      }
      
      return s_SharedDirector;
    }
  }
}
