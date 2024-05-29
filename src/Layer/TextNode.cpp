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
#include "ParagraphParser.hpp"
#include "TransformAttribute.hpp"
#include "StyleItem.hpp"
#include "ParagraphLayout.hpp"
#include "ParagraphPainter.hpp"
#include "VSkFontMgr.hpp"
#include "Renderer.hpp"

#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/VType.hpp"

namespace VGG::layer
{

class TextNode__pImpl
{
  VGG_DECL_API(TextNode)
public:
  TextNode__pImpl(TextNode* api)
    : q_ptr(api)
  {
  }

  ParagraphItemAttributeAccessor* accessor;
  TextNode::EventHandler          paragraphNodeEventHandler;
  TextNode__pImpl(TextNode__pImpl&& p) noexcept = default;
  TextNode__pImpl& operator=(TextNode__pImpl&& p) noexcept = delete;
};

TextNode::TextNode(VRefCnt* cnt, int uniqueID, const std::string& name, std::string guid)
  : PaintNode(cnt, uniqueID, name, VGG_TEXT, std::move(guid), RT_DEFAULT, false)
  , d_ptr(new TextNode__pImpl(this))
{
  auto               t = incRef(transformAttribute());
  Ref<ParagraphItem> poa;
  auto [c, d] = StyleItem::MakeRenderNode(
    nullptr,
    this,
    t,
    [&](VAllocator* alloc, StyleItem* object) -> Ref<GraphicItem>
    {
      poa = ParagraphItem::Make(alloc);
      return poa;
    });
  auto acc = std::make_unique<ParagraphItemAttributeAccessor>(*d, poa);
  d_ptr->accessor = acc.get();
  onSetAccessor(std::move(acc));
  onSetStyleItem(c);
  observe(std::move(c));
}

void TextNode::setTextAnchor(glm::vec2 anchor)
{
  d_ptr->accessor->paragraph()->setAnchor(anchor);
}

void TextNode::setParagraph(
  std::string                utf8,
  std::vector<TextStyleAttr> style,
  std::vector<ParagraphAttr> parStyle)
{
  d_ptr->accessor->paragraph()->setParagraph(
    std::move(utf8),
    std::move(style),
    std::move(parStyle));
}

void TextNode::setParagraphBounds(const Bounds& bounds)
{
  d_ptr->accessor->paragraph()->setParagraphBounds(bounds);
}

void TextNode::setFrameMode(ETextLayoutMode layoutMode)
{
  d_ptr->accessor->paragraph()->setFrameMode(layoutMode);
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  d_ptr->accessor->paragraph()->setVerticalAlignment(vertAlign);
}

void TextNode::installParagraphNodeEventHandler(EventHandler handler)
{
  d_ptr->paragraphNodeEventHandler = std::move(handler);
}

void TextNode::dispatchEvent(void* event)
{
  if (d_ptr->paragraphNodeEventHandler)
  {
    d_ptr->paragraphNodeEventHandler(
      static_cast<ParagraphItemAttributeAccessor*>(attributeAccessor()),
      event);
  }
}

void TextNode::onPaint(Renderer* renderer)
{
  auto       canvas = renderer->canvas();
  const auto clip = (overflow() == OF_HIDDEN || overflow() == OF_SCROLL);
  if (clip)
  {
    canvas->save();
    auto bounds = makeBoundsPath();
    bounds.clip(canvas, SkClipOp::kIntersect);
  }
  PaintNode::onPaint(renderer);
  if (clip)
  {
    canvas->restore();
  }
}

Bounds TextNode::onRevalidate(Revalidation* inv, const glm::mat3& mat)
{
  auto b =
    d_ptr->accessor->paragraph()->revalidate(); // We just revalidate the paragraph attribute so far
  PaintNode::onRevalidate(inv, mat);
  return b.bounds(transform());
}

TextNode::~TextNode() = default;

} // namespace VGG::layer
