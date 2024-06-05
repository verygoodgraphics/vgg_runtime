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
#include "Layer/Core/RasterNode.hpp"
#include "Layer/Raster.hpp"

namespace VGG::layer
{

class EmptyRaster : public RasterNode
{
public:
  EmptyRaster(VRefCnt* cnt, Ref<RenderNode> node)
    : RasterNode(cnt, nullptr, nullptr, nullptr, std::move(node))
  {
  }

  void raster(const std::vector<Bounds>& bounds)
  {
  }

  void render(Renderer* renderer)
  {
    getChild()->render(renderer);
  }

  VGG_CLASS_MAKE(EmptyRaster);
};

namespace raster
{

Ref<RasterNode> makeEmptyRaster(Ref<RenderNode> renderObject)
{
  if (!renderObject)
    return nullptr;
  return EmptyRaster::Make(std::move(renderObject));
}
} // namespace raster

} // namespace VGG::layer
