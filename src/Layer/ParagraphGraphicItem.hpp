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
#include "Layer/AttributeNode.hpp"
#include "Layer/Core/VType.hpp"
#include "GraphicItem.hpp"
#include "ParagraphPainter.hpp"
namespace VGG::layer
{

class ParagraphItem : public GraphicItem
{
public:
  ParagraphItem(VRefCnt* cnt);
  virtual void    render(Renderer* renderer) override;
  ShapeAttribute* shape() const override
  {
    return nullptr;
  }

  sk_sp<SkImageFilter> getMaskFilter() const override
  {
    return 0;
  }

  void setParagraph(
    std::string                text,
    std::vector<TextStyleAttr> style,
    std::vector<ParagraphAttr> parStyle)
  {
    if (text.empty())
      return;
    if (style.empty())
      style.push_back(TextStyleAttr());
    if (parStyle.empty())
      parStyle.push_back(ParagraphAttr());
    m_paragraphLayout->setText(std::move(text));
    m_paragraphLayout->setTextStyle(std::move(style));
    m_paragraphLayout->setLineStyle(std::move(parStyle));
  }

  // VGG_ATTRIBUTE(ParagraphBounds, Bounds, m_fixedBounds);

  void setParagraphBounds(const Bounds& bounds)
  {
    m_paragraphLayout->setParagraphHintBounds(bounds);
  }

  const Bounds& getParagraphBounds() const
  {
    return m_paragraphLayout->getParagraphHintBounds();
  }

  void setVerticalAlignment(ETextVerticalAlignment vertAlign)
  {
    m_paragraphLayout->setVerticalAlignment(vertAlign);
  }

  const ETextVerticalAlignment& getVerticalAlignment() const
  {
    return m_paragraphLayout->getVerticalAlignment();
  }

  void setFrameMode(ETextLayoutMode layoutMode)
  {
    m_paragraphLayout->setTextLayoutMode(layoutMode);
  }

  const ETextLayoutMode& getFrameMode() const
  {
    return m_paragraphLayout->getTextLayoutMode();
  }

  void setAnchor(const glm::vec2& anchor)
  {
    if (m_anchor && *m_anchor == anchor)
      return;
    m_anchor = anchor;
    invalidate();
  }

  Bounds effectBounds() const override
  {
    return bounds();
  }

  const glm::vec2& getAnchor() const
  {
    static glm::vec2 s_dft(0.0f);
    if (m_anchor)
    {
      return *m_anchor;
    }
    return s_dft;
  }

  VGG_CLASS_MAKE(ParagraphItem);

  Bounds onRevalidate() override;

private:
  VParagraphPainterPtr     m_painter;
  Bounds                   m_fixedBounds;
  RichTextBlockPtr         m_paragraphLayout;
  ETextLayoutMode          m_layoutMode;
  std::optional<glm::vec2> m_anchor;
};

} // namespace VGG::layer
