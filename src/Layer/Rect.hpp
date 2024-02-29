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

#include "Layer/Core/Shape.hpp"
#include <core/SkRect.h>

namespace VGG::layer
{

class RectShape final : public Shape
{
public:
  RectShape(const SkRect& rect)
  {
    m_rect = rect;
    setEmpty(rect.isEmpty());
    setClosed(true);
  }

  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    if (m_matrix.isIdentity())
      canvas->drawRect(rect(), paint);
    {
      canvas->save();
      canvas->concat(m_matrix);
      canvas->drawRect(rect(), paint);
      canvas->restore();
    }
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    if (m_matrix.isIdentity())
      canvas->clipRect(rect(), clipOp);
    else
    {
      // canvas->clipPath(asPath(), clipOp);
    }
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
  SkRect   m_rect;
  SkMatrix m_matrix;
  // std::variant<SkRect, SkPath> m_rect;
};

class RRectShape final : public Shape
{
public:
  RRectShape(const SkRRect& rect)
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
    // canvas->clipRect(rrect().rect(), clipOp);
    canvas->clipRRect(rrect(), clipOp);
  }

  SkRect bound() override
  {
    return rrect().rect();
  }

  // void transform(const SkMatrix& matrix) override
  // {
  //   auto     rect = matrix.mapRect(m_rect.rect());
  //   auto     ul = m_rect.radii(SkRRect::Corner::kUpperLeft_Corner);
  //   auto     ur = m_rect.radii(SkRRect::Corner::kUpperRight_Corner);
  //   auto     br = m_rect.radii(SkRRect::Corner::kLowerRight_Corner);
  //   auto     bl = m_rect.radii(SkRRect::Corner::kLowerLeft_Corner);
  //   SkVector radii[4] = { ul, ur, br, bl };
  //   for (int i = 0; i < 4; i++)
  //   {
  //     radii[i].fX = matrix.mapRadius(radii[i].fX);
  //     radii[i].fY = matrix.mapRadius(radii[i].fY);
  //   }
  //   m_rect.setRectRadii(rect, radii);
  //   setEmpty(m_rect.isValid());
  // }

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
