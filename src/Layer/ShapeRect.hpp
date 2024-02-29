/*
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

#include "Layer/Core/VShape.hpp"
#include <core/SkRect.h>

namespace VGG::layer
{

class ShapeRect final : public Shape
{
public:
  ShapeRect(const SkRect& rect)
  {
    m_rect = rect;
    setEmpty(rect.isEmpty());
    setClosed(true);
  }

  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawRect(rect(), paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipRect(rect(), clipOp);
  }

  SkRect bound() override
  {
    return rect();
  }

  SkPath asPath() override
  {
    return SkPath::Rect(rect());
  }

  const SkRect& rect() const
  {
    return m_rect;
  }

  SkRect outset(float x, float y)
  {
    auto r = rect();
    r.outset(x, y);
    return r;
  }

private:
  SkRect m_rect;
};

class ShapeRoundedRect final : public Shape
{
public:
  ShapeRoundedRect(const SkRRect& rect)
  {
    m_rect = rect;
    setEmpty(!m_rect.isValid());
    setClosed(true);
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawRRect(rrect(), paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipRRect(rrect(), clipOp);
  }

  SkRect bound() override
  {
    return rrect().rect();
  }

  SkPath asPath() override
  {
    SkPath path;
    path.addRRect(m_rect);
    return path;
  }

  const SkRRect& rrect() const
  {
    return m_rect;
  }

  SkRRect outset(float x, float y) const
  {
    auto r = rrect();
    r.outset(x, y);
    return r;
  }

private:
  SkRRect m_rect;
};
} // namespace VGG::layer
