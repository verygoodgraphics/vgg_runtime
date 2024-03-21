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
#include "ObjectAttribute.hpp"

namespace VGG::layer
{

class LayerFXAttribute : public ImageFilterAttribute
{
public:
  LayerFXAttribute(VRefCnt* cnt, Ref<StyleObjectAttribute> styleObjectAttr)
    : ImageFilterAttribute(cnt)
    , m_styleObjectAttr(styleObjectAttr)
  {
    observe(m_styleObjectAttr);
  }
  VGG_ATTRIBUTE(Blurs, std::vector<Blur>, m_blurs);
  Bound                onRevalidate() override;
  sk_sp<SkImageFilter> getImageFilter() const override
  {
    return m_imageFilter;
  }
  VGG_CLASS_MAKE(LayerFXAttribute);

private:
  friend class RenderNode;
  SkRect revalidateLayerImageFilter(const SkRect& bounds);

  Ref<StyleObjectAttribute> m_styleObjectAttr;
  std::vector<Blur>         m_blurs;
  sk_sp<SkImageFilter>      m_imageFilter;
};

} // namespace VGG::layer
