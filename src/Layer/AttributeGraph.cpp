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
#include "AttributeGraph.hpp"
#include "Mask.hpp"
#include "ObjectAttribute.hpp"

#include "Layer/Renderer.hpp"
#include <core/SkCanvas.h>

namespace VGG::layer
{

void StyleObjectAttribute::draw(Renderer* renderer)
{
  auto filled = m_objectAttr->hasFill();
  // ensureStyleObjectRecorder(skPath, mask, blender, style.fills, style.borders);
  if (filled)
  {
    // ensureDropShadowEffects(style.dropShadow, skPath);
    // dropShadowEffects->render(renderer, skPath);
  }

  // styleDisplayList->render(renderer);

  if (filled)
  {
    // ensureInnerShadowEffects(style.innerShadow);
    // innerShadowEffects->render(renderer, skPath);
  }
}

std::pair<sk_sp<SkImageFilter>, SkRect> AlphaMaskAttribute::evalAlphaMaskFilter(
  sk_sp<SkImageFilter> input,
  const SkRect&        objectBound,
  const MaskMap&       maskObjects)
{
  if (!m_alphaMasks.empty())
  {
    auto alphaMaskIter = AlphaMaskIterator(m_alphaMasks);
    auto layerBound = input ? input.get()->computeFastBounds(objectBound) : objectBound;
    auto layerFilter =
      MaskBuilder::makeAlphaMaskWith(input, m_maskNode, maskObjects, alphaMaskIter, layerBound, 0);

    return { layerFilter, layerBound };
  }
  return {};
}

// Bound StyleObjectAttribute::onRevalidate()
// {
// }

} // namespace VGG::layer
