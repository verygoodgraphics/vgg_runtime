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
#include "Layer/Core/VNode.hpp"
#include "core/SkRefCnt.h"
#include "Layer/Renderer.hpp"

#include <core/SkBBHFactory.h>
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

  Frame__pImpl(Frame* api)
    : q_ptr(api)
  {
  }

  void ensurePicture(Renderer* renderer, const Bound& clipBound)
  {
    if (cache)
      return;
    ASSERT(root);
    SkPictureRecorder rec;
    const auto&       b = q_ptr->bound();
    auto              rt = SkRTreeFactory();
    auto pictureCanvas = rec.beginRecording(clipBound.width(), clipBound.height(), &rt);
    if (enableToOrigin)
    {
      pictureCanvas->translate(-b.topLeft().x, -b.topLeft().y);
    }
    renderer->draw(pictureCanvas, root);
    cache = rec.finishRecordingAsPicture();
    ASSERT(cache);
  }
};

PaintNode* Frame::root() const
{
  ASSERT(d_ptr->root);
  return d_ptr->root.get();
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

void Frame::render(Renderer* renderer, const Bound* clipBound)
{
  VGG_IMPL(Frame);
  revalidate();
  if (clipBound)
    _->ensurePicture(renderer, bound());
  else
    _->ensurePicture(renderer, *clipBound);
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
  return _->root->revalidate().bound(_->root->transform());
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
