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

#include "Layer/Core/SceneNode.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Renderer.hpp"
#include "Utility/Log.hpp"

#include <core/SkColor.h>
#include <core/SkPictureRecorder.h>

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
  FrameArray       frames;
  sk_sp<SkPicture> picture;

  sk_sp<SkPicture> revalidatePicture(const SkRect& bounds)
  {
    SkPictureRecorder rec;
    auto              rt = SkRTreeFactory();
    auto              pictureCanvas = rec.beginRecording(bounds, &rt);
    for (const auto& root : frames)
    {
      pictureCanvas->save();
      pictureCanvas->concat(toSkMatrix(root->transform().matrix()));
      pictureCanvas->drawPicture(root->picture());
      pictureCanvas->restore();
    }
    return rec.finishRecordingAsPicture();
  }
};

SceneNode::SceneNode(VRefCnt* cnt, std::vector<FramePtr> frames)
  : RenderNode(cnt, EState::INVALIDATE)
  , d_ptr(new SceneNode__pImpl(this))
{
  d_ptr->frames = std::move(frames);
  for (auto& frame : d_ptr->frames)
  {
    observe(frame);
  }
}
SceneNode::~SceneNode()
{
  for (auto& frame : d_ptr->frames)
  {
    unobserve(frame);
  }
}

void SceneNode::addFrame(FramePtr frame)
{
  if (frame)
  {
    d_ptr->frames.push_back(std::move(frame));
    observe(d_ptr->frames.back());
    invalidate();
  }
}

void SceneNode::setFrames(const std::vector<layer::FramePtr>& frames)
{
  if (d_ptr->frames == frames)
  {
    return;
  }
  d_ptr->frames = frames;
  for (auto& frame : d_ptr->frames)
  {
    observe(frame);
  }
  invalidate();
}

const std::vector<FramePtr>& SceneNode::getFrames() const
{
  return d_ptr->frames;
}

void SceneNode::insertFrame(int index, FramePtr frame)
{
  if (!frame)
    return;
  if (index < 0 || index > (int)d_ptr->frames.size())
  {
    return;
  }
  if (auto it = d_ptr->frames.insert(d_ptr->frames.begin() + index, frame);
      it != d_ptr->frames.end())
  {
    observe(*it);
    invalidate();
  }
}

void SceneNode::eraseFrame(int index)
{
  if (index < 0 || index > (int)d_ptr->frames.size())
  {
    return;
  }
  if (auto it = d_ptr->frames.erase(d_ptr->frames.begin() + index); it != d_ptr->frames.end())
  {
    unobserve(*it);
    invalidate();
  }
}

void SceneNode::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  ASSERT(d_ptr->picture);
  if (d_ptr->picture)
  {
    canvas->drawPicture(d_ptr->picture.get());
  }
}

#ifdef VGG_LAYER_DEBUG
void SceneNode::debug(Renderer* render)
{
  auto canvas = render->canvas();
  ASSERT(canvas);
  SkPaint strokePen;
  strokePen.setStyle(SkPaint::kStroke_Style);
  strokePen.setColor(SK_ColorCYAN);
  strokePen.setStrokeWidth(3);
  canvas->drawRect(toSkRect(bounds()), strokePen);
  for (auto& frame : d_ptr->frames)
  {
    frame->debug(render);
  }
}
#endif

void SceneNode::nodeAt(int x, int y, NodeVisitor visitor, void* userData)
{
  for (auto& root : d_ptr->frames)
  {
    root->nodeAt(x, y, visitor, userData);
  }
}

PaintNode* SceneNode::nodeByID(int id)
{
  for (auto& root : d_ptr->frames)
  {
    if (auto node = root->nodeByID(id); node)
      return node;
  }
  return nullptr;
}

Bounds SceneNode::effectBounds() const
{
  return bounds();
}

Bounds SceneNode::onRevalidate()
{
  Bounds bounds;
  for (auto& frame : d_ptr->frames)
  {
    bounds.unionWith(frame->revalidate());
  }
  d_ptr->picture = d_ptr->revalidatePicture(toSkRect(bounds));
  return bounds;
}

glm::mat3 SceneNode::getMatrix() const
{
  return glm::mat3{ 1.f };
}

SkPicture* SceneNode::picture() const
{
  return d_ptr->picture.get();
}

} // namespace VGG::layer
