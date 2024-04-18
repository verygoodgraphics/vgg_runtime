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

#include "Layer/Core/RasterCache.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/LayerCache.h"
#include "Renderer.hpp"
#include "Settings.hpp"

#include "Layer/Core/SceneNode.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Timer.hpp"
#include "encode/SkPngEncoder.h"
#include "Utility/Log.hpp"

#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <encode/SkPngEncoder.h>
#include <core/SkSurface.h>
#include <core/SkImage.h>

#include <gpu/GpuTypes.h>
#include <gpu/GrDirectContext.h>
#include <gpu/GrRecordingContext.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <memory>
#include <string>
#include <fstream>
#include <variant>

namespace VGG::layer
{

class SceneNode__pImpl
{
  VGG_DECL_API(SceneNode);

public:
  SceneNode__pImpl(SceneNode* api)
    : q_ptr(api)
  {
  }

  using FrameArray = std::vector<layer::FramePtr>;

  FrameArray frames;
};

SceneNode::SceneNode(VRefCnt* cnt)
  : RenderNode(cnt, EState::INVALIDATE)
  , d_ptr(new SceneNode__pImpl(this))
{
}
SceneNode::~SceneNode() = default;

void SceneNode::setFrames(std::vector<layer::FramePtr> roots)
{
  d_ptr->frames = std::move(roots);
  invalidate();
}

void SceneNode::insertFrame(int index, FramePtr frame)
{
  if (index < 0 || index > (int)d_ptr->frames.size())
  {
    return;
  }
  d_ptr->frames.insert(d_ptr->frames.begin() + index, frame);
  invalidate();
}

void SceneNode::eraseFrame(int index)
{
  if (index < 0 || index > (int)d_ptr->frames.size())
  {
    return;
  }
  d_ptr->frames.erase(d_ptr->frames.begin() + index);
  invalidate();
}

layer::Frame* SceneNode::frame(int index)
{

  return d_ptr->frames[index].get();
}

void SceneNode::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  canvas->drawPicture(nullptr); // TODO
}

void SceneNode::nodeAt(int x, int y, layer::PaintNode::NodeVisitor visitor)
{
  for (auto& root : d_ptr->frames)
  {
    root->nodeAt(x, y, visitor);
  }
}

} // namespace VGG::layer
