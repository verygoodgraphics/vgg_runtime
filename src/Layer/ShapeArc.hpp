/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Layer/Shape.hpp"
#include "Layer/Core/VShape.hpp"
#include <core/SkCanvas.h>

namespace VGG::layer
{

class ArcShape final : public Shape
{
public:
  ArcShape(const SkRect& oval, float startAngle, float sweepAngle, bool useCenter)
  {
    m_oval = oval;
    m_startAngle = startAngle;
    m_sweepAngle = sweepAngle;
    m_useCenter = useCenter;
    setEmpty(false);
    setClosed(true);
  }

  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawArc(m_oval, m_startAngle, m_sweepAngle, m_useCenter, paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipRect(m_oval, clipOp);
  }

  SkPath asPath() override
  {
    SkPath path;
    path.addOval(m_oval);
    return path;
  }

  Shape* clone() const override
  {
    return new ArcShape(m_oval, m_startAngle, m_sweepAngle, m_useCenter);
  }

  SkRect bounds() override
  {
    return m_oval;
  }

  std::optional<SkRect> visualBounds() override
  {
    return m_oval;
  }

  SkRect ellipse() const
  {
    return m_oval;
  }

private:
  SkRect m_oval;
  float  m_startAngle;
  float  m_sweepAngle;
  float  m_useCenter;
};
} // namespace VGG::layer
