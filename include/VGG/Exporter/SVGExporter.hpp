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
#include "Type.hpp"
#include <nlohmann/json.hpp>

namespace VGG::exporter
{
class IteratorImplBase;
class SVGIterator
{
  std::unique_ptr<IteratorImplBase> d_impl; // NOLINT
public:
  SVGIterator(nlohmann::json design, nlohmann::json layout, Resource resource);
  SVGIterator(
    nlohmann::json      design,
    nlohmann::json      layout,
    Resource            resource,
    const ExportOption& opt,
    BuilderResult&      result);
  bool           next(std::string& key, std::vector<char>& data);
  IteratorResult next();
  SVGIterator(const SVGIterator&) = delete;
  SVGIterator& operator=(const SVGIterator&) = delete;
  SVGIterator(SVGIterator&& other) noexcept;
  SVGIterator& operator=(SVGIterator&& other) noexcept = delete;
  ~SVGIterator();
};
} // namespace VGG::exporter
