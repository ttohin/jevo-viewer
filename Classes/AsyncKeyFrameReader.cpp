
#include "AsyncKeyFrameReader.h"

namespace jevo
{
  bool AsyncKeyFrameReader::ReadFromFile(const std::string& fileName,
                                         CellId& initialCellId,
                                         BufferTypePtr& buffer)
  {
    std::ifstream i(fileName);
    if (!i)
      return false;
    
    nlohmann::json json;
    i >> json;
    
    if (json.is_null())
      return false;
    
    PixelPos width = json["width"];
    PixelPos height = json["height"];
    
    width = std::ceil(width / 50.f) * 50;
    height = std::ceil(height / 50.f) * 50;
    
    buffer = std::make_shared<BufferType>(width, height);
    buffer->ForEach([](const int& x, const int& y, GreatPixel& value)
                      {
                        value.pos = Vec2(x, y);
                      });
    
    for (const auto& item : json["region"])
    {
      PixelPos colorValue = item["c"];
      PixelPos x = item["x"];
      PixelPos y = item["y"];
      
      x -= 1;
      y -= 1;
      
      if (colorValue != 0)
      {
        GreatPixel* bufferItem = nullptr;
        if (!buffer->Get(x, y, &bufferItem))
          return false;
        
        auto color = graphic::ColorFromUint(colorValue);
        bufferItem->color = color;
        bufferItem->id = initialCellId++;
      }
    }
    
    return true;
  }

}
