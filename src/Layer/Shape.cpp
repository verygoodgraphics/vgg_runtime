#include "Layer/Core/Shape.hpp"
#include <pathops/SkPathOps.h>
#include "Layer/VSkia.hpp"

#include "Layer/Path.hpp"
#include "Layer/Rect.hpp"
#include "Layer/Ellipse.hpp"

namespace VGG::layer
{
void ShapePath::op(const ShapePath& shape, EBoolOp op)
{
  SkPath p = asPath();
  auto   sop = toSkPathOp(op);
  Op(p, shape.asPath(), sop, &p);
  setPath(p);
}

ShapePath::~ShapePath()
{
}

bool ShapePath::isEmpty() const
{
  return m_type == EMPTY || (m_type == PATH && m_impl->isEmpty());
}

void ShapePath::setPath(const SkPath& path)
{
  m_impl = std::make_shared<ContourShape>(path);
  m_type = PATH;
}

void ShapePath::setContour(const ContourPtr& contour)
{
  m_impl = std::make_shared<ContourShape>(contour);
  m_type = PATH;
}

void ShapePath::setRect(const SkRect& rect)
{
  m_impl = std::make_shared<RectShape>(rect);
  m_type = RECT;
}

std::optional<SkRect> ShapePath::asRect() const
{
  if (type() == RECT)
  {
    return m_impl->asPath().getBounds();
  }
  return std::nullopt;
}

void ShapePath::setRRect(const SkRRect& rrect)
{
  m_impl = std::make_shared<RRectShape>(rrect);
  m_type = RRECT;
}

std::optional<SkRRect> ShapePath::asRRect() const
{
  if (type() == RRECT)
  {
    return std::static_pointer_cast<RRectShape>(m_impl)->rrect();
  }
  return std::nullopt;
}

void ShapePath::setArc(const SkRect& oval, float startAngle, float sweepAngle, bool forceMoveTo)
{
  m_impl = std::make_shared<ArcShape>(oval, startAngle, sweepAngle, forceMoveTo);
  m_type = ARCH;
}
void ShapePath::setOval(const Ellipse& ellipse)
{
  m_impl = std::make_shared<EllipseShape>(ellipse.rect);
  m_type = OVAL;
}

std::optional<Ellipse> ShapePath::asOval() const
{
  if (type() == OVAL)
    return std::static_pointer_cast<EllipseShape>(m_impl)->ellipse();
  return std::nullopt;
}

std::optional<ShapePath> ShapePath::outset(float x, float y) const
{
  switch (type())
  {
    case RECT:
      return ShapePath(static_cast<RectShape*>(m_impl.get())->outset(x, y));
    case RRECT:
      return ShapePath(static_cast<RRectShape*>(m_impl.get())->outset(x, y));
    case OVAL:
      return ShapePath(static_cast<EllipseShape*>(m_impl.get())->outset(x, y));
    default:
      return std::nullopt;
  }
  return std::nullopt;
}
void ShapePath ::setFillType(EWindingType fillType)
{
  if (this->type() == PATH)
  {
    std::static_pointer_cast<ContourShape>(m_impl)->setFillType(fillType);
  }
}

void ShapePath::transform(ShapePath& shape, const SkMatrix& matrix)
{
  SkPath path;
  if (m_type != EMPTY)
  {
    path = m_impl->asPath();
    path.transform(matrix);
    shape = ShapePath(path);
  }
}
} // namespace VGG::layer
