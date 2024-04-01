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
#include "Layer/Core/Attrs.hpp"
#include "VSkia.hpp"
#include "LayerAttribute.hpp"
#include "Effects.hpp"

#include <core/SkImageFilter.h>
#include <core/SkRect.h>

namespace VGG::layer
{

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
                    { filter = makeRadialBlurFilter(blur, m_styleObjectAttr->bound()); } },
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
  m_styleObjectAttr->revalidate();
  const auto styledObjectBounds = toSkRect(m_styleObjectAttr->effectBounds());
  const auto layerBound = revalidateLayerImageFilter(styledObjectBounds);
  return Bounds{ layerBound.x(), layerBound.y(), layerBound.width(), layerBound.height() };
}
} // namespace VGG::layer
