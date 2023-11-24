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
#include "DebugCanvas.hpp"

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
    m_canvas->drawRect(r.rect, paint);
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
    m_canvas->drawLine(rect.fLeft, rect.fTop, rect.fLeft, rect.fBottom, paint);
  }
  else
  {
    m_canvas->drawLine(rect.fLeft, rect.fTop, rect.fRight, rect.fTop, paint);
  }
}
