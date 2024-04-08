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
#include "Utility/Log.hpp"
#include "Layer/Memory/VAllocator.hpp"
#include <cstdlib>

namespace VGG::layer
{

class AllocatorImpl : public VGG::layer::VAllocator
{
public:
  int         count = 0;
  const char* name{ nullptr };
  AllocatorImpl(const char* name = nullptr)
    : name(name)
  {
  }
  void dealloc(void* ptr) override
  {
    count--;
    free(ptr);
  }
  void* alloc(size_t size) override
  {
    count++;
    return malloc(size);
  }
  ~AllocatorImpl() override
  {
    if (count > 0)
    {
      WARN("Maybe memory leak in %s, %d VObjects are not released", name, count);
    }
  }
};
static AllocatorImpl g_globalLayerMemoryAllocator{ "Global Memory Allocator" };
VAllocator*          getGlobalMemoryAllocator()
{
  return nullptr;
  // return &g_globalLayerMemoryAllocator;
}

} // namespace VGG::layer
