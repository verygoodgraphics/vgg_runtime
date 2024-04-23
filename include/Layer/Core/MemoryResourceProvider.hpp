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
#include "Layer/Core/ResourceProvider.hpp"
#include <vector>
#include <string>
#include <unordered_map>
namespace VGG::layer
{
class MemoryResourceProvider : public ResourceProvider
{
public:
  MemoryResourceProvider(std::unordered_map<std::string, std::vector<char>> data);
  MemoryResourceProvider(const MemoryResourceProvider&) = delete;
  MemoryResourceProvider& operator=(const MemoryResourceProvider&) = delete;
  MemoryResourceProvider(MemoryResourceProvider&&) = default;
  MemoryResourceProvider& operator=(MemoryResourceProvider&&) = default;

  void merge(std::unordered_map<std::string, std::vector<char>> data)
  {
    m_data.merge(std::move(data));
  }

  void purge();
  Blob readData(std::string_view guid) override;

private:
  std::unordered_map<std::string, std::vector<char>> m_data;
};
} // namespace VGG::layer
