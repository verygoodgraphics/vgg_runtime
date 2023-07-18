#include "Core/TextNode.h"
#include "Core/Attrs.h"
#include "Core/FontManager.h"
#include "Core/Node.hpp"
#include "Core/PaintNode.h"
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
#include <modules/skparagraph/include/ParagraphCache.h>
#include <string_view>

namespace VGG
{

TextNode::TextNode(const std::string& name)
  : PaintNode(name, VGG_TEXT)
  , d_ptr(new TextNode__pImpl(this))
{
}

void TextNode::setParagraph(std::string utf8,
                            const std::vector<TextAttr>& attrs,
                            const std::vector<TextLineAttr>& lineAttr)
{
  VGG_IMPL(TextNode);
  std::vector<ParagraphAttr> paraAttrs;
  _->text = std::move(utf8);
  if (_->text.empty())
    return;
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
  ParagraphParser parser;
  parser.parse(_->m_paragraphCache, _->text, attrs, paraAttrs);
}

void TextNode::setFrameMode(ETextLayoutMode mode)
{
  VGG_IMPL(TextNode)
  _->mode = mode;
}

void TextNode::paintEvent(SkCanvas* canvas)
{
  VGG_IMPL(TextNode);
  if (_->m_paragraphCache.empty())
    return;
  if (_->m_paragraphCache.test(TextParagraphCache::TextParagraphCacheFlagsBits::D_REBUILD))
  {
    _->m_paragraphCache.rebuild();
    _->m_paragraphCache.clear(TextParagraphCache::TextParagraphCacheFlagsBits::D_REBUILD);
  }
  if (_->m_paragraphCache.test(TextParagraphCache::TextParagraphCacheFlagsBits::D_LAYOUT))
  {
    setBound(_->m_paragraphCache.layout(getBound(), _->mode)); // update bound
    _->m_paragraphCache.clear(TextParagraphCache::TextParagraphCacheFlagsBits::D_LAYOUT);
  }

  PaintNode::paintEvent(canvas);
  canvas->save();
  // we need to convert to skia coordinate to render text
  canvas->scale(1, -1);
  {
    const auto bound = getBound();
    int totalHeight = _->m_paragraphCache.getHeight();
    int curY = 0;
    if (_->m_vertAlign == ETextVerticalAlignment::VA_Bottom)
    {
      curY = bound.height() - totalHeight;
    }
    else if (_->m_vertAlign == ETextVerticalAlignment::VA_Center)
    {
      curY = (bound.height() - totalHeight) / 2;
    }
    for (int i = 0; i < _->m_paragraphCache.paragraphCache.size(); i++)
    {
      auto& p = _->m_paragraphCache.paragraphCache[i].paragraph;
      const auto curX = _->m_paragraphCache.paragraphCache[i].offsetX;
      p->paint(canvas, curX, curY);
      auto lastLine = p->lineNumber();
      if (lastLine < 1)
        continue;
      LineMetrics lineMetric;
      p->getLineMetricsAt(lastLine - 1, &lineMetric);
      if (Scene::isEnableDrawDebugBound())
      {
        DebugCanvas debugCanvas(canvas);
        drawParagraphDebugInfo(debugCanvas,
                               _->m_paragraphCache.paragraph[i],
                               p.get(),
                               curX,
                               curY,
                               i);
      }
      curY += p->getHeight() - lineMetric.fHeight;
    }
  }
  canvas->restore();
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  VGG_IMPL(TextNode);
  _->m_vertAlign = vertAlign;
}

TextNode::~TextNode() = default;

} // namespace VGG
