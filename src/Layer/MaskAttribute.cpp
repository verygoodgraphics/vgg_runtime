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
// #include "MaskAttribute.hpp"
#include "Layer/ImageFilterAttribute.hpp"
#include "VSkia.hpp"
#include "MaskAttribute.hpp"
#include "LayerAttribute.hpp"
#include "Mask.hpp"

#include "Layer/Core/PaintNode.hpp"

namespace VGG::layer
{

AlphaMaskAttribute::AlphaMaskAttribute(
  VRefCnt*                  cnt,
  PaintNode*                maskedNode,
  Ref<ImageFilterAttribute> inputFilter)
  : ImageFilterAttribute(cnt)
  , m_inputFilter(inputFilter)
  , m_maskedNode(maskedNode)
  , m_maskMap(nullptr)
{
  observe(inputFilter);
}

void AlphaMaskAttribute::setInputImageFilter(Ref<ImageFilterAttribute> input)
{
  if (input != m_inputFilter)
  {
    unobserve(m_inputFilter);
    observe(input);
    m_inputFilter = input;
    invalidate();
  }
}

Bound AlphaMaskAttribute::onRevalidate()
{
  auto layerBound = toSkRect(m_inputFilter->revalidate());
  if (!m_alphaMasks.empty() && m_maskMap)
  {
    auto     layerFilter = m_inputFilter->getImageFilter();
    auto     alphaMaskIter = AlphaMaskIterator(m_alphaMasks);
    SkMatrix resetOffset = SkMatrix::Translate(
      layerBound.x(),
      layerBound.y()); // note that rasterized shader is located in the origin, we need a matrix
    m_alphaMaskFilter = MaskBuilder::makeAlphaMaskWith(
      layerFilter,
      m_maskedNode,
      *m_maskMap,
      alphaMaskIter,
      layerBound,
      &resetOffset);
    return Bound{ layerBound.x(), layerBound.y(), layerBound.width(), layerBound.height() };
  }
  return Bound();
}

Bound ShapeMaskAttribute::onRevalidate()
{
  if (!m_maskID.empty() && m_maskMap)
  {
    auto iter = ShapeMaskIterator(m_maskID);
    m_shape = MaskBuilder::makeShapeMask(
      m_maskedNode,
      *m_maskMap,
      iter,
      toSkRect(m_maskedNode->frameBound()), // This bound is inccorect, layer bound should be used
      0);
  }
  return Bound();
}

} // namespace VGG::layer
