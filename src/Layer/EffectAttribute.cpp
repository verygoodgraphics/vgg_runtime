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
#include "VSkia.hpp"
#include "EffectAttribute.hpp"
#include "ObjectAttribute.hpp"
#include "Effects.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/StyleItem.hpp"

#include <core/SkImageFilter.h>
#include <core/SkRect.h>

namespace VGG::layer
{

LayerFXAttribute::LayerFXAttribute(VRefCnt* cnt, WeakRef<StyleItem> styleItem)
  : ImageFilterAttribute(cnt)
{
  m_styleItem = styleItem;
}

SkRect LayerFXAttribute::revalidateLayerImageFilter(const SkRect& bounds)
{
  if (!m_blurs.empty())
  {
    for (const auto& b : m_blurs)
    {
      if (!b.isEnabled)
        continue;
      sk_sp<SkImageFilter> filter;
      std::visit(
        Overloaded{ [&](const GaussianBlur& blur) { filter = makeLayerBlurFilter(blur); },
                    [&](const MotionBlur& blur) { filter = makeMotionBlurFilter(blur); },
                    [&, this](const RadialBlur& blur)
                    {
                      auto styleItem = m_styleItem.lock();
                      if (styleItem)
                        filter = makeRadialBlurFilter(blur, styleItem->objectBounds());
                    } },
        b.type);
      if (m_imageFilter == nullptr)
      {
        m_imageFilter = filter;
      }
      else
      {
        m_imageFilter = SkImageFilters::Compose(m_imageFilter, filter);
      }
    }
    return m_imageFilter ? m_imageFilter->computeFastBounds(bounds) : bounds;
  }
  return bounds;
}

Bounds LayerFXAttribute::onRevalidate()
{
  auto styleItem = m_styleItem.lock();
  if (styleItem)
  {
    styleItem->revalidate(); // cycle revalidation !!!: workaround
    const auto styledObjectBounds = toSkRect(styleItem->styleEffectBounds());
    const auto layerBounds = revalidateLayerImageFilter(styledObjectBounds);
    return Bounds{ layerBounds.x(), layerBounds.y(), layerBounds.width(), layerBounds.height() };
  }
  return Bounds();
}

Bounds BackdropFXAttribute::onRevalidate()
{
  if (!m_blurs.empty())
  {
    for (const auto& b : m_blurs)
    {
      if (!b.isEnabled)
        continue;
      sk_sp<SkImageFilter> filter;
      filter = makeBackgroundBlurFilter(b.blur);
      if (m_imageFilter == nullptr)
      {
        m_imageFilter = filter;
      }
      else
      {
        m_imageFilter = SkImageFilters::Compose(m_imageFilter, filter);
      }
    }
  }
  return Bounds();
}

} // namespace VGG::layer
