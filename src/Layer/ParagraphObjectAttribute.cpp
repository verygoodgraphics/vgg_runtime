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
#include "ParagraphObjectAttribute.hpp"
#include "Layer/AttributeAccessor.hpp"
namespace VGG::layer
{

ParagraphObjectAttribute::ParagraphObjectAttribute(VRefCnt* cnt)
  : InnerObjectAttribute(cnt)
{
  m_paragraphLayout = makeRichTextBlockPtr(VGGFontCollection::GlobalFontCollection());
  m_painter = makeVParagraphPainterPtr();
  m_painter->setParagraph(m_paragraphLayout);
  observe(m_painter);
}
void ParagraphObjectAttribute::render(Renderer* renderer)
{
  if (m_paragraphLayout->empty())
    return;
  // auto canvas = renderer->canvas();
  // const auto clip = (q_ptr->overflow() == OF_HIDDEN || q_ptr->overflow() == OF_SCROLL);
  // if (clip)
  // {
  //   canvas->save();
  //   auto bound = q_ptr->makeBoundPath();
  //   bound.clip(canvas, SkClipOp::kIntersect);
  // }
  if (m_anchor && !m_paragraphLayout->paragraphCache.empty())
  {
    auto offsetY = m_anchor->y - m_paragraphLayout->firstBaseline();
    m_painter->paintRaw(renderer, m_anchor->x, offsetY);
  }
  else
  {
    m_painter->paintParagraph(renderer);
  }
  // if (clip)
  // {
  //   canvas->restore();
  // }
}

Bounds ParagraphObjectAttribute::onRevalidate()
{
  return m_painter->revalidate();
}

} // namespace VGG::layer
