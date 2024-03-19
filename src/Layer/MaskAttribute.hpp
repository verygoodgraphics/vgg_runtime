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
#include "ImageFilterAttribute.hpp"

namespace VGG::layer
{

class AlphaMaskAttribute : public ImageFilterAttribute
{
public:
  using MaskMap = std::unordered_map<std::string, PaintNode*>;
  AlphaMaskAttribute(VRefCnt* cnt, PaintNode* maskedNode)
    : ImageFilterAttribute(cnt)
    , m_maskedNode(maskedNode)
  {
  }

  VGG_CLASS_MAKE(AlphaMaskAttribute);
  VGG_ATTRIBUTE_PTR(MaskNode, PaintNode, m_maskedNode);
  VGG_ATTRIBUTE(AlphaMasks, std::vector<AlphaMask>, m_alphaMasks);

  Bound onRevalidate() override
  {
    return Bound();
  }

private:
  std::pair<sk_sp<SkImageFilter>, SkRect> evalAlphaMaskFilter(
    sk_sp<SkImageFilter> input,
    const SkRect&        crop,
    const MaskMap&       maskObjects);
  sk_sp<SkShader>        m_alphaMaskShader;
  std::vector<AlphaMask> m_alphaMasks;
  PaintNode*             m_maskedNode;
};
} // namespace VGG::layer
