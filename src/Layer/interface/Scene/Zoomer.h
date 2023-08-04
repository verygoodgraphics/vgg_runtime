#pragma once
#include "Common/Math.hpp"
#include <SDL2/SDL.h>

using namespace VGG;

class SkCanvas;

struct Zoomer
{
  static constexpr double minZoom = 0.01;
  static constexpr double maxZoom = 10.0;

  Vec2 offset{ 0.0, 0.0 };
  double zoom{ 1.0 };
  bool panning{ false };
  double dpiRatio{ 1.0 };

  void apply(SkCanvas* canvas);
  void restore(SkCanvas* canvas);
  SDL_Event mapEvent(SDL_Event evt, double scaleFactor);
  void mapWindowPosToLogicalPosition(const float windowXY[2], float scaleFactor, float logicXY[2]);
};
