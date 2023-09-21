#pragma once

#include "core/SkCanvas.h"
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

namespace sktxt = skia::textlayout;
class DebugCanvas
{
public:
  DebugCanvas(SkCanvas* canvas)
    : canvas(canvas)
  {
  }

  void drawRects(SkColor color, std::vector<sktxt::TextBox>& result, bool fill = false);

  void drawLine(SkColor color, SkRect rect, bool vertical = true);

  void drawLines(SkColor color, std::vector<sktxt::TextBox>& result)
  {

    for (auto& r : result)
    {
      drawLine(color, r.rect);
    }
  }

  SkCanvas* get()
  {
    return canvas;
  }

private:
  SkCanvas* canvas;
  const char* name;
};
