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

#include "Layer/Core/ViewportNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Memory/Ref.hpp"

class GrRecordingContext;
namespace VGG::layer
{

class RasterNode;
class Viewport;
class RenderNode;
class ZoomerNode;

namespace raster
{

Ref<RasterNode> make(
  GrRecordingContext* device,
  Ref<Viewport>       viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     renderObject);

Ref<RasterNode> makeEmptyRaster(Ref<RenderNode> renderObject);

Ref<RasterNode> makeTileRaster(
  GrRecordingContext* device,
  Ref<Viewport>       viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     renderObject);

} // namespace raster
} // namespace VGG::layer
