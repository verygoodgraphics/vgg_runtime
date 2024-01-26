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
#include "Layer/ParagraphParser.hpp"
#include "ParagraphLayout.hpp"
#include "ParagraphPainter.hpp"
#include "AttrSerde.hpp"
#include "VSkFontMgr.hpp"
#include "Renderer.hpp"

#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Painter.hpp"
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
#ifdef USE_SHARED_PTR
    paragraphLayout = makeRichTextBlockPtr(VGGFontCollection::GlobalFontCollection());
#else
    paragraphLayout =
      makeRichTextBlockPtr(getGlobalMemoryAllocator(), VGGFontCollection::GlobalFontCollection());
#endif
#ifdef USE_SHARED_PTR
    painter = makeVParagraphPainterPtr();
#else
    painter = makeVParagraphPainterPtr(getGlobalMemoryAllocator());
#endif
    painter->setParagraph(paragraphLayout);

#ifdef USE_SHARED_PTR
#else
    q_ptr->observe(painter);
#endif
  }

  VParagraphPainterPtr     painter;
  RichTextBlockPtr         paragraphLayout;
  std::optional<glm::vec2> anchor;

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

#ifdef USE_SHARED_PTR
  bool observed{ false };
  void ensureObserve()
  {
    if (!observed)
    {
      q_ptr->observe(painter);
      observed = true;
    }
  }
#else
#endif
};

TextNode::TextNode(VRefCnt* cnt, const std::string& name, std::string guid)
  : PaintNode(cnt, name, VGG_TEXT, std::move(guid))
  , d_ptr(new TextNode__pImpl(this))
{
}

void TextNode::setTextAnchor(glm::vec2 anchor)
{
  VGG_IMPL(TextNode);
  _->anchor = anchor;
}

void TextNode::setParagraph(
  std::string                utf8,
  std::vector<TextStyleAttr> style,
  std::vector<ParagraphAttr> parStyle)
{
  VGG_IMPL(TextNode);
  if (utf8.empty())
    return;
  if (style.empty())
    style.push_back(TextStyleAttr());
  if (parStyle.empty())
    parStyle.push_back(ParagraphAttr());
  _->paragraphLayout->setText(std::move(utf8));
  _->paragraphLayout->setTextStyle(std::move(style));
  _->paragraphLayout->setLineStyle(std::move(parStyle));
}

void TextNode::setFrameMode(ETextLayoutMode layoutMode)
{
  VGG_IMPL(TextNode);
#ifdef USE_SHARED_PTR
  _->ensureObserve();
#endif
  TextLayoutMode mode;
  switch (layoutMode)
  {
    case TL_FIXED:
      mode = TextLayoutFixed(frameBound());
      break;
    case TL_AUTOWIDTH:
      mode = TextLayoutAutoWidth();
      break;
    case TL_AUTOHEIGHT:
      mode = TextLayoutAutoHeight(frameBound().width());
      break;
  }
  _->paragraphLayout->setTextLayoutMode(mode);
}

void TextNode::drawAsAlphaMask(Renderer* renderer, sk_sp<SkBlender> blender)
{
  SkPath path;
  paintFill(renderer, std::move(blender), 0, path);
}

void TextNode::drawRawStyle(Painter& painter, const SkPath& path, sk_sp<SkBlender> blender)
{
  auto renderer = painter.renderer();
  VGG_IMPL(TextNode);
  auto canvas = renderer->canvas();
  if (_->paragraphLayout->empty())
    return;
  if (overflow() == OF_HIDDEN)
  {
    canvas->save();
    canvas->clipPath(makeBoundPath());
  }
  if (_->anchor)
  {
    auto offsetY = _->anchor->y - _->paragraphLayout->firstBaseline();
    _->painter->paintRaw(renderer, _->anchor->x, offsetY);
  }
  else
  {
    _->painter->paintParagraph(renderer);
  }
  if (overflow() == OF_HIDDEN)
  {
    canvas->restore();
  }
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  VGG_IMPL(TextNode);
#ifdef USE_SHARED_PTR
  _->ensureObserve();
#endif
  _->paragraphLayout->setVerticalAlignment(vertAlign);
}

Bound TextNode::onRevalidate()
{
  VGG_IMPL(TextNode);
  return _->painter->revalidate();
}

TextNode::~TextNode() = default;

} // namespace VGG::layer
