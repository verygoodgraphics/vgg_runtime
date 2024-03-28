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
#include "RenderObjectAttribute.hpp"
#include "ParagraphPainter.hpp"
namespace VGG::layer
{

class ParagraphObjectAttribute : public RenderObjectAttribute
{
public:
  ParagraphObjectAttribute(VRefCnt* cnt);
  virtual void    render(Renderer* renderer) override;
  ShapeAttribute* shape() const override
  {
    return nullptr;
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

  void setAnchor(const glm::vec2& anchor)
  {
    if (m_anchor && *m_anchor == anchor)
      return;
    m_anchor = anchor;
    invalidate();
  }

  glm::vec2 getAnchor() const
  {
    return m_anchor.value_or(glm::vec2(0.0f));
  }

  VGG_CLASS_MAKE(ParagraphObjectAttribute);

  Bound onRevalidate() override
  {
    return Bound();
  }

private:
  VParagraphPainterPtr     m_painter;
  RichTextBlockPtr         m_paragraphLayout;
  std::optional<glm::vec2> m_anchor;
};

} // namespace VGG::layer
