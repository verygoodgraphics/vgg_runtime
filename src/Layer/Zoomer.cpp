#include "Utility/Log.h"
#include "VGG/Layer/Zoomer.h"

#include <core/SkCanvas.h>

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
