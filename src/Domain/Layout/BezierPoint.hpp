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

#include "Domain/Layout/Rect.hpp"
#include "Domain/Model/DesignModel.hpp"

#include <nlohmann/json.hpp>

#include <optional>

namespace VGG
{
namespace Layout
{

struct BezierPoint
{
  Layout::Point                point;
  std::optional<Layout::Point> from;
  std::optional<Layout::Point> to;

  static BezierPoint makeFromModel(const Model::PointAttr& model);

  BezierPoint makeFromModelFormat() const;
  BezierPoint makeModelFormat() const;

  BezierPoint makeTransform(const Matrix& matrix) const;
  BezierPoint makeScale(const Rect& oldContainerFrame, const Rect& newContainerFrame) const;
  BezierPoint makeTranslate(const Scalar tx, const Scalar ty) const;

  BezierPoint& scale(const Scalar xScale, const Scalar yScale);
};

// NOLINTBEGIN
void to_json(nlohmann::json& j, const BezierPoint& beziorPoint);
void from_json(const nlohmann::json& j, BezierPoint& beziorPoint);
// NOLINTEND

} // namespace Layout
} // namespace VGG