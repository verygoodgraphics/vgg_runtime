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

// SDL_Event Zoomer::mapEvent(SDL_Event evt, double scaleFactor)
// {
//   if (evt.type == SDL_MOUSEMOTION)
//   {
//     double x1 = evt.motion.x;
//     double y1 = evt.motion.y;
//     double x0 = x1 - evt.motion.xrel;
//     double y0 = y1 - evt.motion.yrel;
//     x1 = (x1 / scaleFactor - offset.x) / m_zoom;
//     y1 = (y1 / scaleFactor - offset.y) / m_zoom;
//     x0 = (x0 / scaleFactor - offset.x) / m_zoom;
//     y0 = (y0 / scaleFactor - offset.y) / m_zoom;
//     evt.motion.x = FLOAT2SINT(x1);
//     evt.motion.y = FLOAT2SINT(y1);
//     evt.motion.xrel = FLOAT2SINT(x1 - x0);
//     evt.motion.yrel = FLOAT2SINT(y1 - y0);
//   }
//   return evt;
// }

Vec2 Zoomer::doTranslate(float x, float y)
{
  offset.x += x;
  offset.y += y;
  return translate();
}
float Zoomer::doZoom(float speed, float x, float y)
{
  double dz = speed;
  double z2 = zoom * (1 + dz);
  if (z2 > 0.01 && z2 < 100)
  {
    offset.x -= (x - offset.x) * dz;
    offset.y -= (y - offset.y) * dz;
    zoom += zoom * dz;
  }
  return scale();
}

void Zoomer::mapCanvasPosToLogicalPosition(const float canvasXY[2], float logicXY[2]) const
{
  float x1 = canvasXY[0];
  float y1 = canvasXY[1];
  x1 = (x1 - offset.x) / zoom;
  y1 = (y1 - offset.y) / zoom;
  logicXY[0] = x1;
  logicXY[1] = y1;
}
