#include <Common/DebugCanvas.h>

void DebugCanvas::drawRects(SkColor color, std::vector<sktxt::TextBox>& result, bool fill)
{

  SkPaint paint;
  if (!fill)
  {
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);
  }
  paint.setColor(color);
  for (auto& r : result)
  {
    canvas->drawRect(r.rect, paint);
  }
}

void DebugCanvas::drawLine(SkColor color, SkRect rect, bool vertical)
{

  SkPaint paint;
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setAntiAlias(true);
  paint.setStrokeWidth(1);
  paint.setColor(color);
  if (vertical)
  {
    canvas->drawLine(rect.fLeft, rect.fTop, rect.fLeft, rect.fBottom, paint);
  }
  else
  {
    canvas->drawLine(rect.fLeft, rect.fTop, rect.fRight, rect.fTop, paint);
  }
}
