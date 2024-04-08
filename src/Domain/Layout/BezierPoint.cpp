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

BezierPoint BezierPoint::makeFromModel(const Model::PointAttr& model)
{
  BezierPoint result;

  auto v = model.point;
  result.point = { v[0], v[1] };
  if (model.curveFrom.has_value())
  {
    auto v = model.curveFrom.value();
    result.from = { v[0], v[1] };
  }
  if (model.curveTo.has_value())
  {
    auto v = model.curveTo.value();
    result.to = { v[0], v[1] };
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

BezierPoint BezierPoint::makeScale(const Rect& oldContainerFrame, const Rect& newContainerFrame)
  const
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

BezierPoint& BezierPoint::scale(const Scalar xScale, const Scalar yScale)
{
  point.scale(xScale, yScale);
  if (from.has_value())
  {
    from = from->scale(xScale, yScale);
  }
  if (to.has_value())
  {
    to = to->scale(xScale, yScale);
  }
  return *this;
}