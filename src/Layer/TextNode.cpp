#include "VSkFontMgr.hpp"
#include "TextNodePrivate.hpp"
#include "Renderer.hpp"

#include "VGG/Layer/Core/TextNode.hpp"
#include "VGG/Layer/Core/Attrs.hpp"
#include "VGG/Layer/Core/Node.hpp"
#include "VGG/Layer/Core/PaintNode.hpp"
#include "VGG/Layer/Core/VType.hpp"
#include "VGG/Layer/FontManager.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/effects/SkRuntimeEffect.h>
#include <core/SkTextBlob.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/ParagraphCache.h>

#include <string_view>
#include <memory>

namespace VGG
{

TextNode::TextNode(const std::string& name, std::string guid)
  : PaintNode(name, VGG_TEXT, std::move(guid))
  , d_ptr(new TextNode__pImpl(this))
{
  auto mgr = sk_sp<SkFontMgrVGG>(FontManager::instance().defaultFontManager());
  if (mgr)
  {
    mgr->ref();
  }
  auto fontCollection = sk_ref_sp(new VGGFontCollection(std::move(mgr)));
  d_ptr->paragraphCache.setFontCollection(fontCollection);
}

TextNode::TextNode(const TextNode& other)
  : PaintNode(other)
  , d_ptr(new TextNode__pImpl(*other.d_ptr))
{
}

NodePtr TextNode::clone() const
{
  auto newNode = std::make_shared<TextNode>(*this);
  return newNode;
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
  parser.parse(_->paragraphCache, _->text, attrs, paraAttrs);
}

void TextNode::setFrameMode(ETextLayoutMode mode)
{
  VGG_IMPL(TextNode)
  _->mode = mode;
}

void TextNode::paintEvent(SkiaRenderer* renderer)
{
  auto canvas = renderer->canvas();
  VGG_IMPL(TextNode);
  if (_->paragraphCache.empty())
    return;
  if (_->paragraphCache.test(TextParagraphCache::ETextParagraphCacheFlagsBits::D_REBUILD))
  {
    _->paragraphCache.rebuild();
    _->paragraphCache.clear(TextParagraphCache::ETextParagraphCacheFlagsBits::D_REBUILD);
  }
  if (_->paragraphCache.test(TextParagraphCache::ETextParagraphCacheFlagsBits::D_LAYOUT))
  {
    setBound(_->paragraphCache.layout(getBound(), _->mode)); // update bound
    _->paragraphCache.clear(TextParagraphCache::ETextParagraphCacheFlagsBits::D_LAYOUT);
  }

  if (overflow() == OF_Hidden)
  {
    canvas->save();
    canvas->clipPath(makeBoundPath());
  }

  canvas->save();
  // we need to convert to skia coordinate to render text
  canvas->scale(1, -1);
  {
    const auto bound = getBound();
    int totalHeight = _->paragraphCache.getHeight();
    int curY = 0;
    if (_->vertAlign == ETextVerticalAlignment::VA_Bottom)
    {
      curY = bound.height() - totalHeight;
    }
    else if (_->vertAlign == ETextVerticalAlignment::VA_Center)
    {
      curY = (bound.height() - totalHeight) / 2;
    }
    for (int i = 0; i < _->paragraphCache.paragraphCache.size(); i++)
    {
      auto& p = _->paragraphCache.paragraphCache[i].paragraph;
      const auto curX = _->paragraphCache.paragraphCache[i].offsetX;
      p->paint(canvas, curX, curY);
      if (Scene::isEnableDrawDebugBound())
      {
        DebugCanvas debugCanvas(canvas);
        drawParagraphDebugInfo(debugCanvas, _->paragraphCache.paragraph[i], p.get(), curX, curY, i);
      }
      auto lastLine = p->lineNumber();
      LineMetrics lineMetric;
      if (lastLine < 1)
        continue;
      p->getLineMetricsAt(lastLine - 1, &lineMetric);
      curY += p->getHeight() - lineMetric.fHeight;
    }
  }
  canvas->restore();

  if (overflow() == OF_Hidden)
  {
    canvas->restore();
  }
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  VGG_IMPL(TextNode);
  _->vertAlign = vertAlign;
}

TextNode::~TextNode() = default;

} // namespace VGG
