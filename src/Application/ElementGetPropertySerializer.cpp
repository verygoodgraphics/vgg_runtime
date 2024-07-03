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

#include "ElementGetPropertySerializer.hpp"
#include <nlohmann/json.hpp>

namespace nlohmann
{
namespace
{
struct ToJsonVisitor
{
  json& j;

  void operator()(const std::monostate&)
  {
    j = nullptr;
  }
  void operator()(bool x)
  {
    j = x;
  }
  void operator()(int x)
  {
    j = x;
  }
  void operator()(double x)
  {
    j = x;
  }
  void operator()(std::size_t x)
  {
    j = x;
  }
  void operator()(const std::string& x)
  {
    j = x;
  }
  void operator()(const VGG::app::ElementColor& x)
  {
    j = x;
  }
  void operator()(const VGG::app::ElementMatrix& x)
  {
    j = x;
  }
  void operator()(const VGG::app::ImageFilter& x)
  {
    j = x;
  }
};

} // namespace

void adl_serializer<VGG::app::ElementProperty>::to_json(json& j, const VGG::app::ElementProperty& x)
{
  std::visit(ToJsonVisitor{ j }, x);
}

} // namespace nlohmann
