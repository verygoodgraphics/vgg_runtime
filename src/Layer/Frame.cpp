/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "Layer/VSkia.hpp"
#include "Settings.hpp"
#include "Layer/Core/Frame.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VNode.hpp"
#include "core/SkRefCnt.h"
#include "Layer/Renderer.hpp"

#include <core/SkBBHFactory.h>
#include <core/SkColor.h>
#include <core/SkSurface.h>
#include <core/SkPictureRecorder.h>

#include <unordered_map>

namespace
{
using namespace VGG::layer;

void nodeAtRecursive(
  PaintNode*               node,
  int                      x,
  int                      y,
  std::vector<PaintNode*>& nodes,
  const glm::mat3&         matrix)
{
  if (node->isVisible())
  {
    const auto m = matrix * node->transform().matrix();
    const auto current = toSkMatrix(m);
    auto       currentBounds = current.mapRect(toSkRect(node->bound()));
    if (currentBounds.contains(x, y))
    {
      nodes.push_back(node);
    }
    for (auto c = node->cbegin(); c != node->cend(); ++c)
    {
      nodeAtRecursive(static_cast<PaintNode*>(c->get()), x, y, nodes, m);
    }
  }
}

PaintNode* nodeAtRecursive(PaintNode* node, int x, int y, const glm::mat3& matrix)
{
  if (node->isVisible())
  {
    const auto m = matrix * node->transform().matrix();
    const auto current = toSkMatrix(m);
    auto       deviceBounds = current.mapRect(toSkRect(node->frameBound()));
    DEBUG(
      "node: %s [%f %f %f %f ]",
      node->name().c_str(),
      deviceBounds.fLeft,
      deviceBounds.fTop,
      deviceBounds.fRight,
      deviceBounds.fBottom);
    if (deviceBounds.contains(x, y))
    {
      return node;
    }
    for (auto& child : *node)
    {
      return nodeAtRecursive(static_cast<PaintNode*>(child.get()), x, y, m);
    }
  }
  return nullptr;
}

} // namespace

namespace VGG::layer
{
class Frame__pImpl
{
  VGG_DECL_API(Frame)
public:
  PaintNodePtr     root;
  sk_sp<SkPicture> cache;
  bool             enableToOrigin{ false };
  Transform        transform;

  Frame__pImpl(Frame* api)
    : q_ptr(api)
  {
  }

  sk_sp<SkPicture> renderPicture(const SkRect& bound)
  {
    Renderer          r;
    SkPictureRecorder rec;
    auto              rt = SkRTreeFactory();
    auto              pictureCanvas = rec.beginRecording(bound, &rt);
    r.draw(pictureCanvas, root);
    if (getDebugBoundEnable())
    {
      SkPaint paint;
      paint.setColor(SK_ColorBLUE);
      paint.setStyle(SkPaint::kStroke_Style);
      paint.setStrokeWidth(1);
      pictureCanvas->drawRect(bound, paint);
    }
    return rec.finishRecordingAsPicture();
  }
};

PaintNode* Frame::root() const
{
  ASSERT(d_ptr->root);
  return d_ptr->root.get();
}

const Transform& Frame::transform() const
{
  ASSERT(d_ptr->root);
  return d_ptr->transform;
}

const std::string& Frame::guid() const
{
  ASSERT(d_ptr->root);
  return d_ptr->root->guid();
}

bool Frame::isVisible() const
{
  ASSERT(d_ptr->root);
  return d_ptr->root->isVisible();
}

void Frame::resetToOrigin(bool enable)
{
  VGG_IMPL(Frame);
  _->enableToOrigin = enable;
  invalidate();
}

SkPicture* Frame::picture()
{
  return d_ptr->cache.get();
}

Bound Frame::onRevalidate()
{
  VGG_IMPL(Frame);
  ASSERT(_->root);
  auto bounds = root()->revalidate();
  auto b = bounds.bound(root()->transform());
  _->cache = _->renderPicture(toSkRect(b));
  _->transform.setMatrix(glm::mat3{ 1 });
  if (_->enableToOrigin)
  {
    _->transform.setMatrix(
      glm::translate(glm::mat3{ 1 }, glm::vec2(-b.topLeft().x, -b.topLeft().y)));
  }
  return b;
}

void Frame::setClipBound(const Bound& bound)
{
  VGG_IMPL(Frame);
  ASSERT(_->root);
  invalidate();
}

Frame::Frame(VRefCnt* cnt, PaintNodePtr root)
  : VNode(cnt, INVALIDATE)
  , d_ptr(std::make_unique<Frame__pImpl>(this))
{
  ASSERT(root);
  d_ptr->root = root;
  observe(root);
}

PaintNode* Frame::nodeAt(int x, int y)
{
  if (auto r = root(); r)
  {
    ASSERT(root()->parent() == nullptr);
    auto inv = d_ptr->transform.inverse();
    auto p = inv * glm::vec3(x, y, 1);
    auto n = r->nodeAt(p.x, p.y);
    return n;
  }
  return nullptr;
}

void Frame::nodesAt(int x, int y, std::vector<PaintNode*>& nodes)
{
  if (auto r = root(); r)
  {
    auto inv = d_ptr->transform.inverse();
    auto p = inv * glm::vec3(x, y, 1);
    return r->nodesAt(p.x, p.y, nodes);
  }
}

PaintNode* Frame::nodeByID(const std::string& id)
{
  return static_cast<PaintNode*>(root()->findChildRecursive(id).get());
}

Frame::~Frame()
{
}
} // namespace VGG::layer
