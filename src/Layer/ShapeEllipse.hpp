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

namespace VGG::layer
{
class EllipseShape final : public Shape
{
public:
  EllipseShape(const SkRect& rect)
  {
    m_oval = rect;
    setEmpty(false);
    setClosed(true);
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawOval(m_oval, paint);
  }

  SkRect bound() override
  {
    return m_oval;
  }
  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipPath(const_cast<EllipseShape*>(this)->asPath(), clipOp);
  }

  SkPath asPath() override
  {
    if (m_path)
      return *m_path;
    m_path = SkPath::Oval(m_oval);
    return *m_path;
  }

  const SkRect& ellipse() const
  {
    return m_oval;
  }

  SkRect outset(float x, float y)
  {
    auto r = ellipse();
    r.outset(x, y);
    return r;
  }

private:
  SkRect                m_oval;
  std::optional<SkPath> m_path;
};
} // namespace VGG::layer
