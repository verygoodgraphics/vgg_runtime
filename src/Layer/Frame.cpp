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
#include "Layer/PaintRenderNode.hpp"
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

PaintNodeAdaptor* asPaintNode(RenderNode* node)
{
  if (node)
    return static_cast<PaintNodeAdaptor*>(node);
  return nullptr;
}

class Frame__pImpl
{
  VGG_DECL_API(Frame)
public:
  bool             enableToOrigin{ false };
  Transform        transform;
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
  ASSERT(getChild());
  return asPaintNode(getChild())->node();
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

SkPicture* Frame::picture() const
{
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

  // auto b = TransformEffectNode::onRevalidate().bounds(node()->transform());

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
  : TransformEffectNode(cnt, 0, PaintNodeAdaptor::Make(std::move(root)))
  , d_ptr(std::make_unique<Frame__pImpl>(this))
{
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
}
} // namespace VGG::layer
