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

#include "RenderResultAttribute.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"

#include <core/SkCanvas.h>

namespace VGG::layer
{

void RenderResultAttribute::render(Renderer* renderer)
{
  auto                canvas = renderer->canvas();
  SkAutoCanvasRestore acr(canvas, true);
  auto                skMatrix = toSkMatrix(m_transformAttr->getTransform().matrix());
  canvas->concat(skMatrix);
  canvas->drawPicture(m_picture);
}

void RenderResultAttribute::draw(Renderer* renderer)
{
  auto dropbackFilter = m_layerAttr->getPreProcess()->getImageFilter();
  auto layerFilter = m_layerAttr->getPostProcess()->getImageFilter();

  const auto newLayer = dropbackFilter || layerFilter; //|| !_->alphaMaskBy.empty();
  auto       path = m_shapeAttr->getShape();
  VShape     shapeMask = m_shapeMaskAttr->getShape();

  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->save();
    shapeMask.clip(renderer->canvas(), SkClipOp::kIntersect);
  }
  if (newLayer)
  {
    // _->ensureStyleObjectRecorder(path, shapeMask, 0, _->style.fills, _->style.borders);
    // objectBound.join(_->styleDisplayList->bounds());
    // _->ensureDropShadowEffects(_->style.dropShadow, path);
    // objectBound.join(_->dropShadowEffects->bounds());
    // SkRect layerBound =
    //   layerFilter ? layerFilter.get()->computeFastBounds(objectBound) : objectBound;
    //
    // if (!_->alphaMaskBy.empty())
    // {
    //   auto alphaMaskIter = AlphaMaskIterator(_->alphaMaskBy);
    //   layerFilter = MaskBuilder::makeAlphaMaskWith(
    //     layerFilter,
    //     this,
    //     renderer->maskObjects(),
    //     alphaMaskIter,
    //     layerBound,
    //     0);
    // }

    // if (dropbackFilter)
    // {
    //   if (auto df = _->styleDisplayList->asImageFilter(); df)
    //   {
    //     auto blender = getOrCreateBlender("maskOut", g_maskOutBlender);
    //     dropbackFilter = SkImageFilters::Blend(blender, df, dropbackFilter, objectBound);
    //   }
    // }
    SkPaint layerPaint;
    layerPaint.setAntiAlias(true);
    layerPaint.setImageFilter(layerFilter);
    auto   renderBounds = toSkRect(bound());
    VShape clipShape(renderBounds);
    beginLayer(renderer, &layerPaint, &clipShape, dropbackFilter);
  }
  m_styleObjectAttr->draw(renderer);
  if (newLayer)
  {
    endLayer(renderer);
  }

  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->restore();
  }
}

void RenderResultAttribute::beginLayer(
  Renderer*            renderer,
  const SkPaint*       paint,
  const VShape*        clipShape,
  sk_sp<SkImageFilter> backdropFilter)
{
  renderer->canvas()->save();
  if (clipShape)
  {
    clipShape->clip(renderer->canvas(), SkClipOp::kIntersect);
  }
  auto layerBound = clipShape->bounds();
  renderer->canvas()->saveLayer(
    SkCanvas::SaveLayerRec(&layerBound, paint, backdropFilter.get(), 0));
}

void RenderResultAttribute::endLayer(Renderer* renderer)
{
  renderer->canvas()->restore();
  renderer->canvas()->restore();
}

Bound RenderResultAttribute::onRevalidate()
{
  m_shapeAttr->revalidate();
  m_shapeMaskAttr->revalidate();
  m_transformAttr->revalidate();
  auto layerBound = toSkRect(m_layerAttr->revalidate());
  auto styleObjectBound = toSkRect(m_styleObjectAttr->revalidate());
  layerBound.join(styleObjectBound);
  return Bound{ layerBound.x(), layerBound.y(), layerBound.width(), layerBound.height() };
}
} // namespace VGG::layer
