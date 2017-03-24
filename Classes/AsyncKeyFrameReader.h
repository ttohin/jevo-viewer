

#pragma once

#include "WorldModel.h"

namespace jevo
{
  class AsyncKeyFrameReader
  {
  public:
    
    bool ReadFromFile(const std::string& fileName,
                      CellId& initialCellId,
                      BufferTypePtr& buffer);
  };
}
