/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "PathPatch.h"
#include <map>
#include <cassert>
#include <optional>
#include <functional>

// NOLINTBEGIN
namespace VGG::layer
{

// <x, y>
typedef std::pair<double, double> point;

const double pi = 3.141592653589793;

nlohmann::json create_point(
  double                       width,
  double                       height,
  const point&                 actual_point,
  const std::optional<double>& radius = std::nullopt,
  const std::optional<point>&  curve_from = std::nullopt,
  const std::optional<point>&  curve_to = std::nullopt)
{
  nlohmann::json out;
  out["class"] = "pointAttr";

  if (radius)
  {
    out["radius"] = *radius;
  }

  if (curve_from)
  {
    out["curveFrom"].emplace_back(std::get<0>(*curve_from) * width);
    out["curveFrom"].emplace_back(-std::get<1>(*curve_from) * height);
  }

  if (curve_to)
  {
    out["curveTo"].emplace_back(std::get<0>(*curve_to) * width);
    out["curveTo"].emplace_back(-std::get<1>(*curve_to) * height);
  }

  out["point"].emplace_back(std::get<0>(actual_point) * width);
  out["point"].emplace_back(-std::get<1>(actual_point) * height);

  return out;
}

void change_rectangle(nlohmann::json& sub_geometry, double width, double height)
{
  assert(sub_geometry.value("class", "") == "rectangle");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double radius[4] = {};
  auto   it = sub_geometry.find("radius");
  if (it != sub_geometry.end())
  {
    for (int i = 0; i < 4; ++i)
    {
      radius[i] = it->at(i).get<double>();
    }
  }

  out["points"].emplace_back(create_point(width, height, { 0, 0 }, radius[0]));
  out["points"].emplace_back(create_point(width, height, { 1, 0 }, radius[1]));
  out["points"].emplace_back(create_point(width, height, { 1, 1 }, radius[2]));
  out["points"].emplace_back(create_point(width, height, { 0, 1 }, radius[3]));

  sub_geometry = std::move(out);
}

void change_ellipse(nlohmann::json& sub_geometry, double width, double height)
{
  assert(sub_geometry.value("class", "") == "ellipse");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double p1 = 0.22385762510000001;
  double p2 = 0.77614237490000004;
  out["points"].emplace_back(
    create_point(width, height, { 0.5, 1 }, std::nullopt, { { p2, 1 } }, { { p1, 1 } }));
  out["points"].emplace_back(
    create_point(width, height, { 1, 0.5 }, std::nullopt, { { 1, p1 } }, { { 1, p2 } }));
  out["points"].emplace_back(
    create_point(width, height, { 0.5, 0 }, std::nullopt, { { p1, 0 } }, { { p2, 0 } }));
  out["points"].emplace_back(
    create_point(width, height, { 0, 0.5 }, std::nullopt, { { 0, p2 } }, { { 0, p1 } }));

  sub_geometry = std::move(out);
}

void change_polygon(nlohmann::json& sub_geometry, double width, double height)
{
  assert(sub_geometry.value("class", "") == "polygon");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double radius = 0;
  auto   it = sub_geometry.find("radius");
  if (it != sub_geometry.end())
  {
    radius = it->get<double>();
  }

  const int point_count = sub_geometry.at("pointCount").get<int>();
  assert(point_count >= 3);

  for (int i = 0; i < point_count; ++i)
  {
    double angle = -pi / 2 + 2 * i * pi / point_count;
    double x = 0.5 + cos(angle) * 0.5;
    double y = 0.5 + sin(angle) * 0.5;
    out["points"].emplace_back(create_point(width, height, { x, y }, radius));
  }

  sub_geometry = std::move(out);
}

void change_star(nlohmann::json& sub_geometry, double width, double height)
{
  assert(sub_geometry.value("class", "") == "star");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double radius = 0;
  auto   it = sub_geometry.find("radius");
  if (it != sub_geometry.end())
  {
    radius = it->get<double>();
  }

  const int point_count = sub_geometry.at("pointCount").get<int>();
  assert(point_count >= 3);

  double ratio = sub_geometry.at("ratio").get<double>();

  for (int i = 0; i < point_count; ++i)
  {
    double angle = -pi / 2 + 2 * i * pi / point_count;
    double angle2 = angle + pi / point_count;

    double x1 = 0.5 + (cos(angle) * 0.5);
    double y1 = 0.5 + (sin(angle) * 0.5);
    out["points"].emplace_back(create_point(width, height, { x1, y1 }, radius));

    double x2 = 0.5 + (cos(angle2) * 0.5 * ratio);
    double y2 = 0.5 + (sin(angle2) * 0.5 * ratio);
    out["points"].emplace_back(create_point(width, height, { x2, y2 }, radius));
  }

  sub_geometry = std::move(out);
}

const std::map<std::string, std::function<decltype(change_rectangle)>> changer{
  { "rectangle", change_rectangle },
  { "ellipse", change_ellipse },
  { "polygon", change_polygon },
  { "star", change_star },
};

bool path_change(nlohmann::json& path)
{
  assert(path["shape"]["subshapes"].size() == 1);
  auto& sub_geometry = path["shape"]["subshapes"][0]["subGeometry"];

  auto it = changer.find(sub_geometry.value("class", ""));
  if (it == changer.end())
  {
    assert(false);
    return false;
  }

  double width = path["bounds"]["width"].get<double>();
  double height = path["bounds"]["height"].get<double>();
  it->second(sub_geometry, width, height);

  return true;
}
} // namespace VGG::layer
// NOLINTEND
