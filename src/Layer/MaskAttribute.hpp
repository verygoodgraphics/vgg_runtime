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
#pragma once
#include "AttributeNode.hpp"
#include "ImageFilterAttribute.hpp"
#include "Layer/LayerAttribute.hpp"
#include "ShapeAttribute.hpp"

#include <core/SkImageFilter.h>

namespace VGG::layer
{

class PaintNode;
class ObjectAttribute;
class LayerAttribute;
using MaskMap = std::unordered_map<std::string, PaintNode*>;
class AlphaMaskAttribute : public ImageFilterAttribute
{
public:
  AlphaMaskAttribute(VRefCnt* cnt, PaintNode* maskedNode, Ref<ImageFilterAttribute> layerAttribute);

  VGG_CLASS_MAKE(AlphaMaskAttribute);
  VGG_ATTRIBUTE_PTR(MaskNode, PaintNode, m_maskedNode);
  VGG_ATTRIBUTE(AlphaMasks, std::vector<AlphaMask>, m_alphaMasks);
  void                 setInputImageFilter(Ref<ImageFilterAttribute> input);
  Bound                onRevalidate() override;
  sk_sp<SkImageFilter> getImageFilter() const override
  {
    return m_alphaMaskFilter ? m_alphaMaskFilter : m_inputFilter->getImageFilter();
  }

private:
  Ref<ImageFilterAttribute> m_inputFilter;
  std::vector<AlphaMask>    m_alphaMasks;
  PaintNode*                m_maskedNode;
  sk_sp<SkImageFilter>      m_alphaMaskFilter;
};

class ShapeMaskAttribute : public ShapeAttribute
{
public:
  ShapeMaskAttribute(VRefCnt* cnt, PaintNode* node, Ref<LayerFXAttribute> layerFX)
    : ShapeAttribute(cnt)
    , m_layerAttr(layerFX)
    , m_maskedNode(node)
  {
    observe(m_layerAttr);
  }

  VGG_ATTRIBUTE(MaskID, std::vector<std::string>, m_maskID);
  VGG_ATTRIBUTE_PTR(MaskNode, PaintNode, m_maskedNode);
  VGG_CLASS_MAKE(ShapeMaskAttribute);

  Bound onRevalidate() override;

private:
  friend class RenderNode;
  Ref<LayerFXAttribute>    m_layerAttr;
  std::vector<std::string> m_maskID;
  PaintNode*               m_maskedNode;
};
} // namespace VGG::layer
