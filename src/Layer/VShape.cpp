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
#include "Layer/VSkia.hpp"
#include "Layer/Shape.hpp"
#include "Layer/ShapePath.hpp"
#include "Layer/ShapeArc.hpp"
#include "Layer/ShapeRect.hpp"
#include "Layer/ShapeEllipse.hpp"
#include "Layer/Core/VShape.hpp"

#include <new>
#include <pathops/SkPathOps.h>

namespace VGG::layer
{
void VShape::op(const VShape& shape, EBoolOp op)
{
  SkPath p = asPath();
  auto   sop = toSkPathOp(op);
  Op(p, shape.asPath(), sop, &p);
  setPath(p);
}

VShape::VShape()
  : m_shape(nullptr)
  , m_type(EMPTY)
{
}

VShape::~VShape()
{
}

VShape::VShape(const VShape& shape)
{
  *this = shape;
}

VShape& VShape::operator=(const VShape& shape)
{
  if (shape.type() != EMPTY)
  {
    ASSERT(shape.m_shape);
    if (auto s = shape.m_shape->clone(); s)
    {
      m_type = shape.type();
      m_shape.reset(s);
    }
  }
  return *this;
}

void VShape::setPath(const SkPath& path)
{
  m_shape = std::make_unique<ShapePath>(path);
  m_type = PATH;
}

void VShape::setContour(const ContourPtr& contour)
{
  m_shape = std::make_unique<ShapePath>(contour);
  m_type = PATH;
}

void VShape::setRect(const SkRect& rect)
{
  m_shape = std::make_unique<ShapeRect>(rect);
  m_type = RECT;
}

std::optional<SkRect> VShape::asRect() const
{
  if (type() == RECT)
  {
    return m_shape->asPath().getBounds();
  }
  return std::nullopt;
}

void VShape::setRRect(const SkRRect& rrect)
{
  m_shape = std::make_unique<ShapeRoundedRect>(rrect);
  m_type = RRECT;
}

std::optional<SkRRect> VShape::asRRect() const
{
  if (type() == RRECT)
  {
    return static_cast<ShapeRoundedRect*>(m_shape.get())->rrect();
  }
  return std::nullopt;
}

void VShape::setArc(const SkRect& oval, float startAngle, float sweepAngle, bool forceMoveTo)
{
  m_shape = std::make_unique<ArcShape>(oval, startAngle, sweepAngle, forceMoveTo);
  m_type = ARC;
}
void VShape::setOval(const Ellipse& ellipse)
{
  m_shape = std::make_unique<EllipseShape>(ellipse.rect);
  m_type = OVAL;
}

void VShape::clip(SkCanvas* canvas, SkClipOp clipOp) const
{
  ASSERT(type() != EMPTY);
  m_shape->clip(canvas, clipOp);
}

void VShape::draw(SkCanvas* canvas, const SkPaint& paint) const
{
  ASSERT(type() != EMPTY);
  m_shape->draw(canvas, paint);
}

SkPath VShape::asPath() const
{
  if (m_shape == nullptr)
    return SkPath();
  return m_shape->asPath();
}

std::optional<Ellipse> VShape::asOval() const
{
  if (type() == OVAL)
    return static_cast<EllipseShape*>(m_shape.get())->ellipse();
  return std::nullopt;
}

std::optional<VShape> VShape::outset(float x, float y) const
{
  switch (type())
  {
    case RECT:
      return VShape(static_cast<ShapeRect*>(m_shape.get())->outset(x, y));
    case RRECT:
      return VShape(static_cast<ShapeRoundedRect*>(m_shape.get())->outset(x, y));
    case OVAL:
      return VShape(static_cast<EllipseShape*>(m_shape.get())->outset(x, y));
    default:
      return std::nullopt;
  }
  return std::nullopt;
}

void VShape ::setFillType(EWindingType fillType)
{
  if (this->type() == PATH)
  {
    static_cast<ShapePath*>(m_shape.get())->setFillType(fillType);
  }
}

bool VShape::isEmpty() const
{
  return m_type == EMPTY || (m_type == PATH && m_shape->isEmpty());
}

bool VShape::isClosed() const
{
  return m_shape->isClosed();
}

void VShape::reset()
{
  if (type() != EMPTY)
  {
    m_type = EMPTY;
    m_shape.reset();
  }
}

void VShape::transform(VShape& shape, const SkMatrix& matrix)
{
  SkPath path;
  if (m_type != EMPTY && !matrix.isIdentity())
  {
    path = m_shape->asPath();
    path.transform(matrix);
    shape = VShape(path);
  }
  else if (matrix.isIdentity())
  {
    shape = *this;
  }
}

std::variant<ContourPtr, SkRRect, SkRect> makeShape(
  std::array<float, 4> radius,
  const SkRect&        rect,
  float                cornerSmoothing)
{
  if (radius[0] != 0 || radius[1] != 0 || radius[2] != 0 || radius[3] != 0)
  {
    if (cornerSmoothing <= 0.f)
    {
      SkRRect  rrect;
      SkVector radii[4] = { { radius[0], radius[0] },
                            { radius[1], radius[1] },
                            { radius[2], radius[2] },
                            { radius[3], radius[3] } };
      rrect.setRectRadii(rect, radii);
      return rrect;
    }
    else
    {
      glm::vec2 corners[4] = { { rect.x(), rect.y() },
                               { rect.right(), rect.y() },
                               { rect.right(), rect.bottom() },
                               { rect.x(), rect.bottom() } };
      Contour   contour;
      contour.closed = true;
      contour.cornerSmooth = cornerSmoothing;
      for (int i = 0; i < 4; i++)
        contour.emplace_back(corners[i], radius[i], std::nullopt, std::nullopt, std::nullopt);
      return std::make_shared<Contour>(contour);
    }
  }
  else
  {
    return rect;
  }
}

} // namespace VGG::layer
