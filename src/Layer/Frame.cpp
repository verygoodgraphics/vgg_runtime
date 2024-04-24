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
#include "Settings.hpp"
#include "Layer/Core/Frame.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VNode.hpp"
#include "core/SkRefCnt.h"
#include "Layer/Renderer.hpp"
#include "LayerCache.h"

#include <core/SkBBHFactory.h>
#include <core/SkColor.h>
#include <core/SkSurface.h>
#include <core/SkPictureRecorder.h>

#include <unordered_map>

namespace VGG::layer
{

class Frame__pImpl
{
  VGG_DECL_API(Frame)
public:
  bool             enableToOrigin{ false };
  Transform        transform;
  Ref<PaintNode>   node;
  sk_sp<SkPicture> cache;

  bool maskDirty{ true };

  Frame__pImpl(Frame* api)
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
    if (getDebugBoundsEnable())
    {
      SkPaint paint;
      paint.setColor(SK_ColorBLUE);
      paint.setStyle(SkPaint::kStroke_Style);
      paint.setStrokeWidth(1);
      pictureCanvas->drawRect(bounds, paint);
    }
    return rec.finishRecordingAsPicture();
  }
};

PaintNode* Frame::node() const
{
  ASSERT(d_ptr->node);
  return d_ptr->node.get();
}

const Transform& Frame::transform() const
{
  return d_ptr->transform;
}

const std::string& Frame::guid() const
{
  return node()->guid();
}

bool Frame::isVisible() const
{
  return node()->isVisible();
}

void Frame::resetToOrigin(bool enable)
{
  VGG_IMPL(Frame);
  _->enableToOrigin = enable;
  invalidate();
}

void Frame::render(Renderer* renderer)
{
  if (d_ptr->cache)
  {
    auto canvas = renderer->canvas();
    canvas->save();
    canvas->concat(toSkMatrix(d_ptr->transform.matrix()));
    renderer->canvas()->drawPicture(d_ptr->cache.get());
    canvas->restore();
  }
  else
  {
    DEBUG("Frame::render: no picture to render");
  }
}

Bounds Frame::effectBounds() const
{
  DEBUG("Does not support effectBounds for Frame so far");
  return node()->bounds();
}

SkPicture* Frame::picture() const
{
  ASSERT(!isInvalid());
  return d_ptr->cache.get();
}

Bounds Frame::onRevalidate()
{
  VGG_IMPL(Frame);
  if (_->maskDirty)
  {
    updateMaskMap(node());
    _->maskDirty = false;
  }

  auto bounds = node()->revalidate();
  auto b = bounds.bounds(node()->transform());

  _->cache = _->renderPicture(toSkRect(b));
  _->transform.setMatrix(glm::mat3{ 1 });
  if (_->enableToOrigin)
  {
    _->transform.setMatrix(
      glm::translate(glm::mat3{ 1 }, glm::vec2(-b.topLeft().x, -b.topLeft().y)));
  }
  return b;
}

Frame::Frame(VRefCnt* cnt, PaintNodePtr root)
  // TransformEffectNode is used just for its RenderNode interface now,
  // real implementation is in Frame class.
  : TransformEffectNode(cnt, 0, 0)
  , d_ptr(std::make_unique<Frame__pImpl>(this))
{
  d_ptr->node = std::move(root);
  observe(d_ptr->node);
}

void Frame::nodeAt(int x, int y, PaintNode::NodeVisitor visitor)
{
  if (auto r = node(); r)
  {
    ASSERT(node()->parent() == nullptr);
    auto inv = d_ptr->transform.inverse();
    auto p = inv * glm::vec3(x, y, 1);
    r->nodeAt(p.x, p.y, visitor);
  }
}

void Frame::invalidateMask()
{
  d_ptr->maskDirty = true;
}

PaintNode* Frame::nodeByID(const std::string& id)
{
  return static_cast<PaintNode*>(node()->findChildRecursive(id).get());
}

Frame::~Frame()
{
  unobserve(d_ptr->node);
}

} // namespace VGG::layer
