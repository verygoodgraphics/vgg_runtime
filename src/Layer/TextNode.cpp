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
#include "Layer/AttributeAccessor.hpp"
#include "Layer/ParagraphObjectAttribute.hpp"
#include "Layer/ParagraphParser.hpp"
#include "Layer/PaintNodePrivate.hpp"
#include "Layer/TransformAttribute.hpp"
#include "ParagraphLayout.hpp"
#include "ParagraphPainter.hpp"
#include "VSkFontMgr.hpp"
#include "Renderer.hpp"

#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/VType.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/effects/SkRuntimeEffect.h>
#include <core/SkTextBlob.h>

#include <memory>

namespace VGG::layer
{
constexpr bool TEXT_LEGACY_CODE = false;

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

  void onDrawRawStyleImpl(Renderer* renderer, sk_sp<SkBlender> blender)
  {
    if (!TEXT_LEGACY_CODE)
    {
      ASSERT(false && "unreachable");
    }
    if (paragraphLayout->empty())
      return;
    auto       canvas = renderer->canvas();
    const auto clip = (q_ptr->overflow() == OF_HIDDEN || q_ptr->overflow() == OF_SCROLL);
    if (clip)
    {
      canvas->save();
      auto bound = q_ptr->makeBoundPath();
      bound.clip(canvas, SkClipOp::kIntersect);
    }
    if (anchor && !paragraphLayout->paragraphCache.empty())
    {
      auto offsetY = anchor->y - paragraphLayout->firstBaseline();
      painter->paintRaw(renderer, anchor->x, offsetY);
    }
    else
    {
      painter->paintParagraph(renderer);
    }
    if (clip)
    {
      canvas->restore();
    }
  }

  VParagraphPainterPtr     painter;
  RichTextBlockPtr         paragraphLayout;
  std::optional<glm::vec2> anchor;

  ParagraphAttributeAccessor* accessor;
  TextNode::EventHandler      paragraphNodeEventHandler;

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
  : PaintNode(cnt, name, VGG_TEXT, std::move(guid), TEXT_LEGACY_CODE, false)
  , d_ptr(new TextNode__pImpl(this))
{
  if (!TEXT_LEGACY_CODE)
  {
    auto                          t = incRef(transformAttribute());
    Ref<ParagraphObjectAttribute> poa;
    auto [c, d] = RenderNodeFactory::MakeDefaultRenderNode(
      nullptr,
      this,
      t,
      [&](VAllocator* alloc, ObjectAttribute* object) -> Ref<RenderObjectAttribute>
      {
        poa = ParagraphObjectAttribute::Make(alloc);
        return poa;
      });

    auto acc = std::make_unique<ParagraphAttributeAccessor>(*d, poa);
    d_ptr->accessor = acc.get();
    PaintNode::d_ptr->accessor = std::move(acc);
    PaintNode::d_ptr->renderNode = std::move(c);
    observe(PaintNode::d_ptr->renderNode);
  }
  // d_ptr->paragraphNodeEventHandler = [this](ParagraphAttributeAccessor*, void*)
  // { DEBUG("text node: %s", this->name().c_str()); };
}

void TextNode::setTextAnchor(glm::vec2 anchor)
{
  VGG_IMPL(TextNode);
  if (TEXT_LEGACY_CODE)
  {
    _->anchor = anchor;
  }
  else
  {
    d_ptr->accessor->paragraph()->setAnchor(anchor);
  }
}

void TextNode::setParagraph(
  std::string                utf8,
  std::vector<TextStyleAttr> style,
  std::vector<ParagraphAttr> parStyle)
{
  VGG_IMPL(TextNode);
  if (TEXT_LEGACY_CODE)
  {
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
  else
  {
    d_ptr->accessor->paragraph()->setParagraph(
      std::move(utf8),
      std::move(style),
      std::move(parStyle));
  }
}

Bound TextNode::getPragraphBound() const
{
  if (TEXT_LEGACY_CODE)
  {
    return PaintNode::frameBound();
  }
  else
  {
    return d_ptr->accessor->paragraph()->getParagraphBound();
  }
}

void TextNode::setParagraphBound(const Bound& bound)
{
  PaintNode::setFrameBound(bound);
  d_ptr->accessor->paragraph()->setParagraphBound(bound);
}

void TextNode::setFrameBound(const Bound& bound)
{
  PaintNode::setFrameBound(bound);
  if (!TEXT_LEGACY_CODE)
  {
    d_ptr->accessor->paragraph()->setParagraphBound(bound);
  }
}

void TextNode::setFrameMode(ETextLayoutMode layoutMode)
{
  VGG_IMPL(TextNode);
  if (TEXT_LEGACY_CODE)
  {
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
  else
  {
    d_ptr->accessor->paragraph()->setFrameMode(layoutMode);
  }
}

void TextNode::onDrawAsAlphaMask(Renderer* renderer, sk_sp<SkBlender> blender)
{
  d_ptr->onDrawRawStyleImpl(renderer, std::move(blender));
}

void TextNode::onDrawStyle(
  Renderer*        renderer,
  const VShape&    path,
  const VShape&    mask,
  sk_sp<SkBlender> blender)
{
  d_ptr->onDrawRawStyleImpl(renderer, std::move(blender));
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  VGG_IMPL(TextNode);
#ifdef USE_SHARED_PTR
  _->ensureObserve();
#endif
  if (TEXT_LEGACY_CODE)
    _->paragraphLayout->setVerticalAlignment(vertAlign);
  else
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
      static_cast<ParagraphAttributeAccessor*>(attributeAccessor()),
      event);
  }
}

Bound TextNode::onRevalidate()
{
  VGG_IMPL(TextNode);
  if (TEXT_LEGACY_CODE)
  {
    auto b = _->painter->revalidate();
    PaintNode::onRevalidate();
    return b;
  }
  else
  {
    auto b = d_ptr->accessor->paragraph()
               ->revalidate(); // We just revalidate the paragraph attribute so far
    PaintNode::onRevalidate();
    return b;
  }
}

TextNode::~TextNode() = default;

} // namespace VGG::layer
