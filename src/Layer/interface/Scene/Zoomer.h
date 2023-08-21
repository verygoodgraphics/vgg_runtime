#pragma once
#include "Common/Math.hpp"

using namespace VGG;

class SkCanvas;

struct Zoomer
{
  static constexpr double minZoom = 0.01;
  static constexpr double maxZoom = 10.0;

  double m_zoom{ 1.0 };
  Vec2 offset{ 0.0, 0.0 };
  bool panning{ false };
  double dpiRatio{ 1.0 };

  // TODO:: remove
  void apply(SkCanvas* canvas);
  void restore(SkCanvas* canvas);

  Vec2 doTranslate(float x, float y);
  float doZoom(float speed, float x, float y);
  float zoom() const
  {
    return m_zoom;
  }

  Vec2 translate() const
  {
    return offset;
  }

  // SDL_Event mapEvent(SDL_Event evt, double scaleFactor);
  void mapWindowPosToLogicalPosition(const float windowXY[2],
                                     float scaleFactor,
                                     float logicXY[2]) const;
};
