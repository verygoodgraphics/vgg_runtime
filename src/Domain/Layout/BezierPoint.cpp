#include "BezierPoint.hpp"

#include "JsonKeys.hpp"
#include "Helper.hpp"

using namespace VGG;
using namespace Layout;

void Layout::to_json(nlohmann::json& j, const BezierPoint& bezierPoint)
{
  j[K_POINT] = bezierPoint.point;

  if (bezierPoint.from.has_value())
  {
    j[K_CURVE_FROM] = bezierPoint.from.value();
  }
  if (bezierPoint.to.has_value())
  {
    j[K_CURVE_TO] = bezierPoint.to.value();
  }
}

void Layout::from_json(const nlohmann::json& j, BezierPoint& bezierPoint)
{
  if (j.contains(K_POINT))
  {
    bezierPoint.point = j[K_POINT];
  }

  if (j.contains(K_CURVE_FROM))
  {
    bezierPoint.from = j[K_CURVE_FROM];
  }

  if (j.contains(K_CURVE_TO))
  {
    bezierPoint.to = j[K_CURVE_TO];
  }
}

BezierPoint BezierPoint::makeTransform(const Matrix& matrix) const
{
  BezierPoint result;

  result.point = point.makeTransform(matrix);
  if (from.has_value())
  {
    result.from = from->makeTransform(matrix);
  }
  if (to.has_value())
  {
    result.to = to->makeTransform(matrix);
  }

  return result;
}

BezierPoint BezierPoint::makeFromModelFormat() const
{
  BezierPoint result;

  result.point = point.makeFromModelPoint();
  if (from.has_value())
  {
    result.from = from->makeFromModelPoint();
  }
  if (to.has_value())
  {
    result.to = to->makeFromModelPoint();
  }

  return result;
}

BezierPoint BezierPoint::makeModelFormat() const
{
  BezierPoint result;

  result.point = point.makeModelPoint();
  if (from.has_value())
  {
    result.from = from->makeModelPoint();
  }
  if (to.has_value())
  {
    result.to = to->makeModelPoint();
  }

  return result;
}

BezierPoint BezierPoint::makeScale(const Rect& oldContainerFrame,
                                   const Rect& newContainerFrame) const
{
  BezierPoint result;

  result.point = point.makeScale(oldContainerFrame, newContainerFrame);
  if (from.has_value())
  {
    result.from = from->makeScale(oldContainerFrame, newContainerFrame);
  }
  if (to.has_value())
  {
    result.to = to->makeScale(oldContainerFrame, newContainerFrame);
  }

  return result;
}

BezierPoint BezierPoint::makeTranslate(const Scalar tx, const Scalar ty) const
{
  BezierPoint result;

  const Point offset{ tx, ty };
  result.point = point + offset;
  if (from.has_value())
  {
    result.from = from.value() + offset;
  }
  if (to.has_value())
  {
    result.to = to.value() + offset;
  }

  return result;
}