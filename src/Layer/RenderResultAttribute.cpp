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
#include "Effects.hpp"

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

std::pair<sk_sp<SkPicture>, Bound> RenderResultAttribute::revalidatePicture()
{
  Renderer  rd;
  Renderer* renderer = &rd;

  auto dropbackFilter = m_styleObjectAttr->getBackgroundBlurImageFilter();
  auto layerFilter = m_layerAttri->getImageFilter();
  auto alphaMaskFilter = m_alphaMaskAttr->getImageFilter();

  const auto newLayer = dropbackFilter || layerFilter || alphaMaskFilter;
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
    SkRect layerBound = toSkRect(m_layerAttri->bound());
    if (alphaMaskFilter)
    {
      layerFilter =
        SkImageFilters::Blend(SkBlendMode::kSrcIn, alphaMaskFilter, layerFilter, layerBound);
    }
    if (dropbackFilter)
    {
      if (auto df = m_styleObjectAttr->asImageFilter(); df)
      {
        auto blender = getOrCreateBlender("maskOut", g_maskOutBlender);
        dropbackFilter =
          SkImageFilters::Blend(blender, df, dropbackFilter, toSkRect(m_styleObjectAttr->bound()));
      }
    }
    SkPaint layerPaint;
    layerPaint.setAntiAlias(true);
    layerPaint.setImageFilter(layerFilter);
    VShape clipShape(layerBound);
    beginLayer(renderer, &layerPaint, &clipShape, dropbackFilter);
  }
  m_styleObjectAttr->render(renderer);
  if (newLayer)
  {
    endLayer(renderer);
  }
  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->restore();
  }
  return { sk_sp<SkPicture>(), bound() };
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
  auto styleObjectBound = toSkRect(m_styleObjectAttr->revalidate());
  auto layerBound = toSkRect(m_layerAttri->revalidate());
  layerBound.join(styleObjectBound);
  auto [pic, bound] = revalidatePicture();
  m_picture = std::move(pic);
  return bound;
}
} // namespace VGG::layer
