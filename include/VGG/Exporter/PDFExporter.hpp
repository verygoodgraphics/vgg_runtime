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

#include <memory>
#include <nlohmann/json.hpp>

namespace VGG::exporter
{
class IteratorImplBase;
namespace pdf
{
class PDFIterator
{
  std::unique_ptr<IteratorImplBase> d_impl; // NOLINT
public:
  PDFIterator(nlohmann::json design, nlohmann::json layout, Resource resource);
  bool next(std::string& key, std::vector<char>& data);
  PDFIterator(const PDFIterator& other) = delete;
  PDFIterator& operator=(const PDFIterator& other) = delete;
  PDFIterator(PDFIterator&& other) noexcept;
  PDFIterator& operator=(PDFIterator&& other) noexcept = delete;
  ~PDFIterator();
};

} // namespace pdf
} // namespace VGG::exporter
