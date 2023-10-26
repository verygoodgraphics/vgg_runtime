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
#pragma once
#include <vector>
#include <optional>
#include <ostream>
namespace VGG
{
class Scene;
}
namespace VGG::layer::exporter
{
namespace pdf
{

struct PDFOptions
{
  int position[2];
  int extend[2];
};

std::optional<std::vector<char>> makePDF(Scene* scene, int page, const PDFOptions& opts);
void makePDF(Scene* scene, int page, const PDFOptions& opts, std::ostream& os);
} // namespace pdf
} // namespace VGG::layer::exporter
