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
#include "Layer/Core/VType.hpp"
#include "Layer/Shape.hpp"
#include "Layer/PathGenerator.hpp"

#include "Layer/Core/VShape.hpp"
#include <core/SkCanvas.h>

namespace VGG::layer
{

class ShapePath final : public Shape
{
public:
  ShapePath(const ContourPtr& contour)
  {
    m_contour = contour;
  }

  ShapePath(const SkPath& path)
  {
    m_contour = nullptr;
    m_path = path;
    setEmpty(m_path->isEmpty());
    setClosed(m_path->isLastContourClosed());
  }

  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    ensurePath();
    if (m_path)
    {
      canvas->drawPath(*m_path, paint);
    }
  }

  SkRect bound() override
  {
    ensurePath();
    if (m_path)
      return m_path->getBounds();
    return SkRect::MakeEmpty();
  }

  Shape* clone() const override
  {
    ensurePath();
    return new ShapePath(*m_path);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    ensurePath();
    if (m_path)
      canvas->clipPath(*m_path, clipOp);
  }

  SkPath asPath() override
  {
    ensurePath();
    if (m_path)
      return *m_path;
    return SkPath();
  }

  void setFillType(EWindingType fillType)
  {
    ensurePath();
    if (m_path)
      m_path->setFillType(
        fillType == EWindingType::WR_EVEN_ODD ? SkPathFillType::kEvenOdd
                                              : SkPathFillType::kWinding);
  }

private:
  ContourPtr            m_contour;
  std::optional<SkPath> m_path;
  void                  ensurePath() const
  {
    const_cast<ShapePath*>(this)->ensurePath();
  }

  void ensurePath()
  {
    if (!m_path && m_contour)
    {
      m_path = VGG::layer::makePath(*m_contour);
      setEmpty(m_path->isEmpty());
      setClosed(m_path->isLastContourClosed());
    }
  }
};
} // namespace VGG::layer
