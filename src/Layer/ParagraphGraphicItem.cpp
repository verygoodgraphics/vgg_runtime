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
#include "ParagraphGraphicItem.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
namespace VGG::layer
{

ParagraphItem::ParagraphItem(VRefCnt* cnt)
  : GraphicItem(cnt)
{
  m_paragraphLayout = makeRichTextBlockPtr(VGGFontCollection::GlobalFontCollection());
  m_painter = makeVParagraphPainterPtr();
  m_painter->setParagraph(m_paragraphLayout);
  observe(m_painter);
}
void ParagraphItem::render(Renderer* renderer)
{
  if (m_paragraphLayout->empty())
    return;
  if (m_anchor && !m_paragraphLayout->paragraphCache.empty())
  {
    auto offsetY = m_anchor->y - m_paragraphLayout->firstBaseline();
    m_painter->paintRaw(renderer, m_anchor->x, offsetY);
  }
  else
  {
    m_painter->paintParagraph(renderer);
  }
}

Bounds ParagraphItem::onRevalidate(Revalidation* inv, const glm::mat3 & mat)
{
  return m_painter->revalidate();
}

} // namespace VGG::layer
