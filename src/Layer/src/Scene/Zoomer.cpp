#include "Scene/Zoomer.h"
#include "core/SkCanvas.h"

void Zoomer::apply(SkCanvas* canvas)
{
  ASSERT(canvas);
  canvas->translate(offset.x, offset.y);
  canvas->scale(zoom, zoom);
}

void Zoomer::restore(SkCanvas* canvas)
{
  ASSERT(canvas);
  canvas->scale(1. / zoom, 1. / zoom);
  canvas->translate(-offset.x, -offset.y);
}

SDL_Event Zoomer::mapEvent(SDL_Event evt, double scaleFactor)
{
  if (evt.type == SDL_MOUSEMOTION)
  {
    double x1 = evt.motion.x;
    double y1 = evt.motion.y;
    double x0 = x1 - evt.motion.xrel;
    double y0 = y1 - evt.motion.yrel;
    x1 = (x1 / scaleFactor - offset.x) / zoom;
    y1 = (y1 / scaleFactor - offset.y) / zoom;
    x0 = (x0 / scaleFactor - offset.x) / zoom;
    y0 = (y0 / scaleFactor - offset.y) / zoom;
    evt.motion.x = FLOAT2SINT(x1);
    evt.motion.y = FLOAT2SINT(y1);
    evt.motion.xrel = FLOAT2SINT(x1 - x0);
    evt.motion.yrel = FLOAT2SINT(y1 - y0);
  }
  return evt;
}

void Zoomer::mapWindowPosToLogicalPosition(const float windowXY[2],
                                           float scaleFactor,
                                           float logicXY[2])
{
  float x1 = windowXY[0];
  float y1 = windowXY[1];
  // float x0 = x1 - windowXYRel[0];
  // float y0 = y1 - windowXYRel[1];
  x1 = (x1 / scaleFactor - offset.x) / zoom;
  y1 = (y1 / scaleFactor - offset.y) / zoom;
  // x0 = (x0 / scaleFactor - offset.x) / zoom;
  // y0 = (y0 / scaleFactor - offset.y) / zoom;
  logicXY[0] = x1;
  logicXY[1] = y1;
  // logicXYRel[0] = x1 - x0;
  // logicXYRel[1] = y1 - y0;
}
