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
#include "Layer/AttrSerde.hpp"
#include "Layer/ParagraphPainter.hpp"
#include "ParagraphLayout.hpp"
#include "VSkFontMgr.hpp"
#include "Renderer.hpp"

#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/FontManager.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/effects/SkRuntimeEffect.h>
#include <core/SkTextBlob.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/ParagraphCache.h>

#include <string_view>
#include <memory>

namespace VGG::layer
{

class TextNode__pImpl
{
  VGG_DECL_API(TextNode)
public:
  TextNode__pImpl(TextNode* api)
    : q_ptr(api)
  {
    paragraphLayout = std::make_shared<RichTextBlock>();
    auto mgr = sk_ref_sp(FontManager::instance().defaultFontManager());
    auto fontCollection = sk_make_sp<VGGFontCollection>(std::move(mgr));
    painter = std::make_shared<VParagraphPainter>();
    painter->setParagraph(paragraphLayout);
  }

  std::shared_ptr<VParagraphPainter> painter;
  std::shared_ptr<RichTextBlock>     paragraphLayout;
  bool                               observed{ false };

  TextNode__pImpl(const TextNode__pImpl& p)
  {
    this->operator=(p);
  }

  TextNode__pImpl& operator=(const TextNode__pImpl& p)
  {
    return *this;
  }

  TextNode__pImpl(TextNode__pImpl&& p) noexcept = default;
  TextNode__pImpl& operator=(TextNode__pImpl&& p) noexcept = delete;

  void ensureObserve()
  {
    if (!observed)
    {
      q_ptr->observe(painter);
      observed = true;
    }
  }
};

TextNode::TextNode(const std::string& name, std::string guid)
  : PaintNode(name, VGG_TEXT, std::move(guid))
  , d_ptr(new TextNode__pImpl(this))
{
}

TextNode::TextNode(const TextNode& other)
  : PaintNode(other)
  , d_ptr(new TextNode__pImpl(*other.d_ptr))
{
}

TreeNodePtr TextNode::clone() const
{
  auto newNode = std::make_shared<TextNode>(*this);
  return newNode;
}

void TextNode::setParagraph(
  std::string                      utf8,
  std::vector<TextStyleAttr>       attrs,
  const std::vector<TextLineAttr>& lineAttr)
{
  VGG_IMPL(TextNode);
  _->ensureObserve();
  std::vector<ParagraphAttr> paraAttrs;
  for (const auto a : lineAttr)
  {
    paraAttrs.emplace_back(a, ETextHorizontalAlignment::HA_Left);
  }
  if (paraAttrs.empty())
  {
    TextLineAttr attr;
    attr.level = 0;
    attr.lineType = TLT_Plain;
    attr.firstLine = false;
    paraAttrs.emplace_back(attr, ETextHorizontalAlignment::HA_Left);
  }

  _->paragraphLayout->setText(std::move(utf8));
  _->paragraphLayout->setTextStyle(std::move(attrs));
  _->paragraphLayout->setLineStyle(std::move(paraAttrs));

  // ParagraphParser parser;
  // parser.parse(_->paragraphCache, _->text, _->textAttr, paraAttrs);
}

void TextNode::setFrameMode(ETextLayoutMode layoutMode)
{
  VGG_IMPL(TextNode);
  _->ensureObserve();
  TextLayoutMode mode;
  switch (layoutMode)
  {
    case TL_Fixed:
      mode = TextLayoutFixed(frameBound());
      break;
    case TL_WidthAuto:
      mode = TextLayoutAutoWidth();
      break;
    case TL_HeightAuto:
      mode = TextLayoutAutoHeight(frameBound().width());
      break;
  }
  _->paragraphLayout->setTextLayoutMode(mode);
}

void TextNode::paintFill(Renderer* renderer, sk_sp<SkBlender> blender, const SkPath& path)
{
  VGG_IMPL(TextNode);
  auto canvas = renderer->canvas();
  if (_->paragraphLayout->empty())
    return;
  if (overflow() == OF_Hidden)
  {
    canvas->save();
    canvas->clipPath(makeBoundPath());
  }
  _->painter->paint(renderer);
  if (overflow() == OF_Hidden)
  {
    canvas->restore();
  }
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  VGG_IMPL(TextNode);
  _->ensureObserve();
  _->paragraphLayout->setVerticalAlignment(vertAlign);
}

Bound TextNode::onRevalidate()
{
  VGG_IMPL(TextNode);
  return _->painter->revalidate();
}

TextNode::~TextNode() = default;

} // namespace VGG::layer
