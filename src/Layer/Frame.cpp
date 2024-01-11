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

  void ensurePicture(Renderer* renderer, const SkMatrix* mat, const Bound& clipBound)
  {
    if (cache)
      return;
    ASSERT(root);
    SkPictureRecorder rec;
    const auto&       b = q_ptr->bound();
    auto              rt = SkRTreeFactory();
    auto              pictureCanvas = rec.beginRecording(
      SkRect::MakeXYWH(b.topLeft().x, b.topLeft().y, b.width(), b.height()),
      &rt);
    if (mat)
      pictureCanvas->setMatrix(*mat);
    renderer->draw(pictureCanvas, root);
    if (renderer->isEnableDrawDebugBound())
    {
      SkPaint paint;
      paint.setColor(SK_ColorBLUE);
      paint.setStyle(SkPaint::kStroke_Style);
      paint.setStrokeWidth(1);
      pictureCanvas->drawRect(toSkRect(b), paint);
    }
    cache = rec.finishRecordingAsPicture();
    ASSERT(cache);
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

void Frame::render(Renderer* renderer, const SkMatrix* mat)
{
  VGG_IMPL(Frame);
  revalidate();
  _->ensurePicture(renderer, mat, bound());
}

SkPicture* Frame::picture()
{
  return d_ptr->cache.get();
}

Bound Frame::onRevalidate()
{
  VGG_IMPL(Frame);
  ASSERT(_->root);
  _->cache = nullptr;
  auto b = _->root->revalidate().bound(_->root->transform());
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

Frame::~Frame()
{
}
} // namespace VGG::layer
