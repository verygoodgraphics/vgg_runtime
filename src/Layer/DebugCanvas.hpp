/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
