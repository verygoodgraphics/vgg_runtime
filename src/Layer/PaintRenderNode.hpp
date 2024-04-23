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

#include "Renderer.hpp"
#include "Settings.hpp"
#include "Layer/Core/RenderNode.hpp"
#include "Layer/Core/PaintNode.hpp"

#include <core/SkPictureRecorder.h>
namespace VGG::layer
{

// Adaptor just for internal use
class PaintNodeAdaptor : public RenderNode
{
public:
  PaintNodeAdaptor(VRefCnt* cnt, PaintNodePtr paintNode)
    : RenderNode(cnt, EState::INVALIDATE)
    , m_paintNode(std::move(paintNode))
  {
    ASSERT(m_paintNode);
    observe(m_paintNode);
  }

  virtual void render(Renderer* render) override
  {
    if (m_picture)
    {
      render->canvas()->drawPicture(m_picture);
    }
  }

  PaintNode* node()
  {
    return m_paintNode.get();
  }

  virtual Bounds effectBounds() const override
  {
    return m_paintNode->bounds();
  }

  virtual SkPicture* picture() const override
  {
    return m_picture.get();
  }

  Bounds onRevalidate() override
  {
    // auto b = m_paintNode->revalidate().bounds(m_paintNode->transform());
    // m_picture = renderPicture(toSkRect(b));
    return Bounds();
  }

  ~PaintNodeAdaptor()
  {
    unobserve(m_paintNode);
  }

  VGG_CLASS_MAKE(PaintNodeAdaptor);

private:
  sk_sp<SkPicture> renderPicture(const SkRect& bounds)
  {
    Renderer          r;
    SkPictureRecorder rec;
    auto              rt = SkRTreeFactory();
    auto              pictureCanvas = rec.beginRecording(bounds, &rt);
    r.draw(pictureCanvas, m_paintNode);
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

  PaintNodePtr     m_paintNode;
  sk_sp<SkPicture> m_picture;
};
} // namespace VGG::layer
