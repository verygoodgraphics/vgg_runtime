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

#include "Layer/Core/ResourceManager.hpp"
#include "Layer/Core/ResourceProvider.hpp"

namespace
{
using namespace VGG::layer;
std::unique_ptr<ResourceProvider> g_provider;
} // namespace

namespace VGG::layer
{

void setGlobalResourceProvider(std::unique_ptr<ResourceProvider> provider)
{
  if (g_provider != provider)
  {
    g_provider = std::move(provider);
  }
}

ResourceProvider* getGlobalResourceProvider()
{
  return g_provider.get();
}
} // namespace VGG::layer
