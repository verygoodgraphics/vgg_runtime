#include "Core/TextNode.h"
#include "Core/FontManager.h"
#include "Core/Node.hpp"
#include "Core/VGGType.h"
#include "Core/VGGUtils.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/effects/SkRuntimeEffect.h"
#include "core/SkTextBlob.h"

#include "Core/TextNodePrivate.h"

#include <memory>
#include <modules/skparagraph/include/FontCollection.h>
#include <string_view>

namespace VGG
{

TextNode::TextNode(const std::string& name)
  : PaintNode(name, VGG_TEXT)
  , d_ptr(new TextNode__pImpl(this))
{
}

void TextNode::setText(const std::string& utf8, const std::vector<TextStyleStub>& styles)
{
  VGG_IMPL(TextNode)
  _->text = utf8;
  _->styles = styles;
}

void TextNode::setTextStyle(size_t position, const TextStyleStub& style)
{
}

void TextNode::setParagraph(const std::string& utf8,
                            const std::vector<TextAttr>& attrs,
                            const std::vector<TextLineAttr>& lineAttr)
{
  VGG_IMPL(TextNode);
  std::vector<ParagraphAttr> paraAttrs;
  for (const auto a : lineAttr)
  {
    paraAttrs.emplace_back(a, ETextHorizontalAlignment::HA_Left);
  }
  if (!paraAttrs.empty())
  {
    ParagraphParser parser;
    parser.parse(_->m_paragraphCache, utf8, attrs, paraAttrs);
  }
}

void TextNode::setFrameMode(ETextLayoutMode mode)
{
  VGG_IMPL(TextNode)
  _->mode = mode;
}

void TextNode::paintEvent(SkCanvas* canvas)
{
  VGG_IMPL(TextNode);
  canvas->save();
  canvas->clipRect(toSkRect(getBound()));
  canvas->scale(1, -1);
  // we need to convert to skia coordinate to render text
  _->m_paragraphCache.drawParagraph(canvas, getBound());
  canvas->restore();
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  VGG_IMPL(TextNode);
  _->m_vertAlign = vertAlign;
}

TextNode::~TextNode() = default;

} // namespace VGG
