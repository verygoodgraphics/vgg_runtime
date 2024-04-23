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

#include "Layer/Core/MemoryResourceProvider.hpp"
#include <core/SkData.h>
#include "Layer/LayerCache.h"

namespace VGG::layer
{

MemoryResourceProvider::MemoryResourceProvider(
  std::unordered_map<std::string, std::vector<char>> data)
  : m_data(std::move(data))
{
  getGlobalImageCache()->purge();
}

void MemoryResourceProvider::purge()
{
  m_data.clear();
  getGlobalImageCache()->purge();
}

Blob MemoryResourceProvider::readData(std::string_view guid)
{
  if (auto it = m_data.find(guid.data()); it != m_data.end())
  {
    return SkData::MakeWithoutCopy(it->second.data(), it->second.size());
  }
  return nullptr;
}
} // namespace VGG::layer
