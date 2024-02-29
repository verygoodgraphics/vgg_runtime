#include "Layer/Core/VShape.hpp"
#include <pathops/SkPathOps.h>
#include "Layer/VSkia.hpp"

#include "Layer/ShapePath.hpp"
#include "Layer/ShapeArc.hpp"
#include "Layer/ShapeRect.hpp"
#include "Layer/ShapeEllipse.hpp"

namespace VGG::layer
{
void VShape::op(const VShape& shape, EBoolOp op)
{
  SkPath p = asPath();
  auto   sop = toSkPathOp(op);
  Op(p, shape.asPath(), sop, &p);
  setPath(p);
}

VShape::~VShape()
{
}

bool VShape::isEmpty() const
{
  return m_type == EMPTY || (m_type == PATH && m_impl->isEmpty());
}

void VShape::setPath(const SkPath& path)
{
  m_impl = std::make_shared<ShapePath>(path);
  m_type = PATH;
}

void VShape::setContour(const ContourPtr& contour)
{
  m_impl = std::make_shared<ShapePath>(contour);
  m_type = PATH;
}

void VShape::setRect(const SkRect& rect)
{
  m_impl = std::make_shared<ShapeRect>(rect);
  m_type = RECT;
}

std::optional<SkRect> VShape::asRect() const
{
  if (type() == RECT)
  {
    return m_impl->asPath().getBounds();
  }
  return std::nullopt;
}

void VShape::setRRect(const SkRRect& rrect)
{
  m_impl = std::make_shared<ShapeRoundedRect>(rrect);
  m_type = RRECT;
}

std::optional<SkRRect> VShape::asRRect() const
{
  if (type() == RRECT)
  {
    return std::static_pointer_cast<ShapeRoundedRect>(m_impl)->rrect();
  }
  return std::nullopt;
}

void VShape::setArc(const SkRect& oval, float startAngle, float sweepAngle, bool forceMoveTo)
{
  m_impl = std::make_shared<ArcShape>(oval, startAngle, sweepAngle, forceMoveTo);
  m_type = ARCH;
}
void VShape::setOval(const Ellipse& ellipse)
{
  m_impl = std::make_shared<EllipseShape>(ellipse.rect);
  m_type = OVAL;
}

std::optional<Ellipse> VShape::asOval() const
{
  if (type() == OVAL)
    return std::static_pointer_cast<EllipseShape>(m_impl)->ellipse();
  return std::nullopt;
}

std::optional<VShape> VShape::outset(float x, float y) const
{
  switch (type())
  {
    case RECT:
      return VShape(static_cast<ShapeRect*>(m_impl.get())->outset(x, y));
    case RRECT:
      return VShape(static_cast<ShapeRoundedRect*>(m_impl.get())->outset(x, y));
    case OVAL:
      return VShape(static_cast<EllipseShape*>(m_impl.get())->outset(x, y));
    default:
      return std::nullopt;
  }
  return std::nullopt;
}
void VShape ::setFillType(EWindingType fillType)
{
  if (this->type() == PATH)
  {
    std::static_pointer_cast<ShapePath>(m_impl)->setFillType(fillType);
  }
}

void VShape::transform(VShape& shape, const SkMatrix& matrix)
{
  SkPath path;
  if (m_type != EMPTY && !matrix.isIdentity())
  {
    path = m_impl->asPath();
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
