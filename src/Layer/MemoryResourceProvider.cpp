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
#include "Layer/LayerCache.h"
#include <core/SkData.h>
#include <core/SkImage.h>
#include <codec/SkCodec.h>

namespace VGG::layer
{

MemoryResourceProvider::MemoryResourceProvider(
  std::unordered_map<std::string, std::vector<char>> data)
  : m_data(std::move(data))
{
  getGlobalImageStackCache()->purge();
}

void MemoryResourceProvider::purge()
{
  m_data.clear();
  getGlobalImageStackCache()->purge();
}

Blob MemoryResourceProvider::readData(std::string_view guid)
{
  if (auto it = m_data.find(guid.data()); it != m_data.end())
  {
    return SkData::MakeWithCopy(it->second.data(), it->second.size());
  }
  return nullptr;
}
} // namespace VGG::layer
