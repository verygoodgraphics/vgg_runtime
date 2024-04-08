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

#include "PathPatch.h"
#include <map>
#include <cassert>
#include <optional>
#include <functional>

namespace VGG::layer
{

// <x, y>
typedef std::pair<double, double> Point;

const double Pi = 3.141592653589793; // NOLINT

nlohmann::json createPoint(
  double                       width,
  double                       height,
  const Point&                 actualPoint,
  const std::optional<double>& radius = std::nullopt,
  const std::optional<Point>&  curveFrom = std::nullopt,
  const std::optional<Point>&  curveTo = std::nullopt)
{
  nlohmann::json out;
  out["class"] = "pointAttr";

  if (radius)
  {
    out["radius"] = *radius;
  }

  if (curveFrom)
  {
    out["curveFrom"].emplace_back(std::get<0>(*curveFrom) * width);
    out["curveFrom"].emplace_back(-std::get<1>(*curveFrom) * height);
  }

  if (curveTo)
  {
    out["curveTo"].emplace_back(std::get<0>(*curveTo) * width);
    out["curveTo"].emplace_back(-std::get<1>(*curveTo) * height);
  }

  out["point"].emplace_back(std::get<0>(actualPoint) * width);
  out["point"].emplace_back(-std::get<1>(actualPoint) * height);

  return out;
}

void changeRectangle(nlohmann::json& subGeometry, double width, double height)
{
  assert(subGeometry.value("class", "") == "rectangle");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double radius[4] = {};
  auto   it = subGeometry.find("radius");
  if (it != subGeometry.end())
  {
    for (int i = 0; i < 4; ++i)
    {
      radius[i] = it->at(i).get<double>();
    }
  }

  out["points"].emplace_back(createPoint(width, height, { 0, 0 }, radius[0]));
  out["points"].emplace_back(createPoint(width, height, { 1, 0 }, radius[1]));
  out["points"].emplace_back(createPoint(width, height, { 1, 1 }, radius[2]));
  out["points"].emplace_back(createPoint(width, height, { 0, 1 }, radius[3]));

  subGeometry = std::move(out);
}

void changeEllipse(nlohmann::json& subGeometry, double width, double height)
{
  assert(subGeometry.value("class", "") == "ellipse");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double p1 = 0.22385762510000001;
  double p2 = 0.77614237490000004;
  out["points"].emplace_back(
    createPoint(width, height, { 0.5, 1 }, std::nullopt, { { p2, 1 } }, { { p1, 1 } }));
  out["points"].emplace_back(
    createPoint(width, height, { 1, 0.5 }, std::nullopt, { { 1, p1 } }, { { 1, p2 } }));
  out["points"].emplace_back(
    createPoint(width, height, { 0.5, 0 }, std::nullopt, { { p1, 0 } }, { { p2, 0 } }));
  out["points"].emplace_back(
    createPoint(width, height, { 0, 0.5 }, std::nullopt, { { 0, p2 } }, { { 0, p1 } }));

  subGeometry = std::move(out);
}

void changePolygon(nlohmann::json& subGeometry, double width, double height)
{
  assert(subGeometry.value("class", "") == "polygon");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double radius = 0;
  auto   it = subGeometry.find("radius");
  if (it != subGeometry.end())
  {
    radius = it->get<double>();
  }

  const int pointCount = subGeometry.at("pointCount").get<int>();
  assert(pointCount >= 3);

  for (int i = 0; i < pointCount; ++i)
  {
    double angle = -Pi / 2 + 2 * i * Pi / pointCount;
    double x = 0.5 + cos(angle) * 0.5;
    double y = 0.5 + sin(angle) * 0.5;
    out["points"].emplace_back(createPoint(width, height, { x, y }, radius));
  }

  subGeometry = std::move(out);
}

void changeStar(nlohmann::json& subGeometry, double width, double height)
{
  assert(subGeometry.value("class", "") == "star");

  nlohmann::json out;
  out["class"] = "contour";
  out["closed"] = true;

  double radius = 0;
  auto   it = subGeometry.find("radius");
  if (it != subGeometry.end())
  {
    radius = it->get<double>();
  }

  const int pointCount = subGeometry.at("pointCount").get<int>();
  assert(pointCount >= 3);

  double ratio = subGeometry.at("ratio").get<double>();

  for (int i = 0; i < pointCount; ++i)
  {
    double angle = -Pi / 2 + 2 * i * Pi / pointCount;
    double angle2 = angle + Pi / pointCount;

    double x1 = 0.5 + (cos(angle) * 0.5);
    double y1 = 0.5 + (sin(angle) * 0.5);
    out["points"].emplace_back(createPoint(width, height, { x1, y1 }, radius));

    double x2 = 0.5 + (cos(angle2) * 0.5 * ratio);
    double y2 = 0.5 + (sin(angle2) * 0.5 * ratio);
    out["points"].emplace_back(createPoint(width, height, { x2, y2 }, radius));
  }

  subGeometry = std::move(out);
}

const std::map<std::string, std::function<decltype(changeRectangle)>> g_changer{
  { "rectangle", changeRectangle },
  { "ellipse", changeEllipse },
  { "polygon", changePolygon },
  { "star", changeStar },
};

bool pathChange(nlohmann::json& path)
{
  assert(path["shape"]["subshapes"].size() == 1);
  auto& subGeometry = path["shape"]["subshapes"][0]["subGeometry"];

  auto it = g_changer.find(subGeometry.value("class", ""));
  if (it == g_changer.end())
  {
    assert(false);
    return false;
  }

  double width = path["bounds"]["width"].get<double>();
  double height = path["bounds"]["height"].get<double>();
  it->second(subGeometry, width, height);

  return true;
}
} // namespace VGG::layer
