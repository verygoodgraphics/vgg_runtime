#pragma once
#include "Common/Math.hpp"

using namespace VGG;

class SkCanvas;

struct Zoomer
{
public:
  double zoom{ 1.0 };
  Vec2 offset{ 0.0, 0.0 };

  [[deprecated("This will be removed in next version")]] bool panning{ false };
  [[deprecated("This will be removed in next version")]] double dpiRatio{ 1.0 };
  [[deprecated("This will be removed in next version")]] void apply(SkCanvas* canvas);
  [[deprecated("This will be removed in next version")]] void restore(SkCanvas* canvas);

  // dx and dy is in the canvas(texture) space.
  Vec2 doTranslate(float dx, float dy);
  // cx and cy is in the canvas(texture) space
  float doZoom(float speed, float cx, float cy);
  float scale() const
  {
    return zoom;
  }

  Vec2 translate() const
  {
    return offset;
  }

  void mapCanvasPosToLogicalPosition(const float canvasXY[2], float logicXY[2]) const;
};
