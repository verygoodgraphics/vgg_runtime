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

#include "Layer/Core/TransformNode.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VNode.hpp"
#include "core/SkRefCnt.h"
#include "Layer/Renderer.hpp"
#include "LayerCache.h"

#include <algorithm>
#include <core/SkBBHFactory.h>
#include <core/SkColor.h>
#include <core/SkSurface.h>
#include <core/SkPictureRecorder.h>
#include <unordered_map>

namespace
{
VGG::layer::PaintNode* findByID(VGG::layer::PaintNode* ptr, int id)
{
  if (ptr->uniqueID() == id)
  {
    return ptr;
  }
  for (const auto& ptr : *ptr)
  {
    auto n = static_cast<VGG::layer::PaintNode*>(ptr.get());
    if (n->uniqueID() == id)
    {
      return n;
    }
    else
    {
      auto r = findByID(n, id);
      if (r)
        return r;
    }
  }
  return nullptr;
}
} // namespace

namespace VGG::layer
{

class FrameNode__pImpl
{
  VGG_DECL_API(FrameNode)
public:
  Ref<PaintNode> node;

  bool maskDirty{ true };

  FrameNode__pImpl(FrameNode* api)
    : q_ptr(api)
  {
  }

  sk_sp<SkPicture> renderPicture(const SkRect& bounds)
  {
    Renderer          r;
    SkPictureRecorder rec;
    auto              rt = SkRTreeFactory();
    auto              pictureCanvas = rec.beginRecording(bounds, &rt);
    r.draw(pictureCanvas, q_ptr->node());
    return rec.finishRecordingAsPicture();
  }
};

PaintNode* FrameNode::node() const
{
  ASSERT(d_ptr->node);
  return d_ptr->node.get();
}

// const Transform& FrameNode::transform() const
// {
//   return d_ptr->transform;
// }

const std::string& FrameNode::guid() const
{
  return node()->guid();
}

bool FrameNode::isVisible() const
{
  return node()->isVisible();
}

void FrameNode::render(Renderer* renderer)
{
  SkAutoCanvasRestore acr(renderer->canvas(), true);
  renderer->canvas()->concat(toSkMatrix(getTransform()->getMatrix()));
  node()->render(renderer);
}

#ifdef VGG_LAYER_DEBUG
void FrameNode::debug(Renderer* render)
{
  auto canvas = render->canvas();
  ASSERT(canvas);
  if (d_ptr->node)
  {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(toSkMatrix(getTransform()->getMatrix()));
    d_ptr->node->debug(render);
  }
}
#endif

Bounds FrameNode::effectBounds() const
{
  DEBUG("Does not support effectBounds for Frame so far");
  return node()->bounds();
}

Bounds FrameNode::onRevalidate(Revalidation* inv, const glm::mat3& ctm)
{
  VGG_IMPL(FrameNode);
  if (_->maskDirty)
  {
    updateMaskMap(node());
    _->maskDirty = false;
  }
  getTransform()->revalidate();
  const auto matrix = getTransform()->getMatrix();
  const auto bounds = node()->revalidate(inv, matrix * ctm);
  //_->cache = _->renderPicture(toSkRect(bounds));
  return bounds.map(matrix);
}

FrameNode::FrameNode(VRefCnt* cnt, Ref<TransformNode> transform, PaintNodePtr root)
  // TransformEffectNode is used just for its RenderNode interface now,
  // real implementation is in Frame class.
  : TransformEffectNode(cnt, std::move(transform), 0)
  , d_ptr(std::make_unique<FrameNode__pImpl>(this))
{
  d_ptr->node = std::move(root);
  observe(d_ptr->node);
#ifdef VGG_LAYER_DEBUG
  dbgInfo = "Frame Node for " + d_ptr->node->guid();
#endif
}

void FrameNode::nodeAt(int x, int y, NodeVisitor vistor, void* userData)
{
  ASSERT(node()->parent() == nullptr);
  if (bounds().contains(x, y))
  {
    auto inv = getTransform()->getInversedMatrix();
    auto p = inv * glm::vec3(x, y, 1);

    const RenderNode::NodeAtContext ctx{ static_cast<int>(p.x), static_cast<int>(p.y), userData };
    vistor(this, &ctx);
  }
}

void FrameNode::invalidateMask()
{
  d_ptr->maskDirty = true;
}

PaintNode* FrameNode::nodeByID(int id)
{
  if (auto r = node(); r)
  {
    return findByID(r, id);
  }
  return nullptr;
}

FrameNode::~FrameNode()
{
  unobserve(d_ptr->node);
}

} // namespace VGG::layer
