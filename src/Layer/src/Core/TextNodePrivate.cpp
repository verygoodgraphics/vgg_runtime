#include "Core/TextNodePrivate.h"
#include "Core/FontManager.h"
#include "Core/TextNode.h"

#include <core/SkCanvas.h>
#include <core/SkColor.h>

namespace VGG
{

void TextNode__pImpl::drawText(SkCanvas* canvas)
{
  int styleIndex = 0;
  ParagraphStyle style;
  skia::textlayout::TextStyle txtStyle;
  const auto& b = q_ptr->getBound();

  const auto& s = styles[0];
  txtStyle.setColor(SK_ColorBLACK);
  txtStyle.setFontFamilies({ SkString(s.fontName), SkString("Noto Color Emoji") });
  txtStyle.setFontSize(s.size);
  txtStyle.setColor(s.fillColor);
  txtStyle.setDecoration(TextDecoration::kLineThrough);
  style.setTextStyle(txtStyle);

  style.setTextAlign(TextAlign::kCenter);
  style.setEllipsis(u"...");

  auto fontCollection = getDefaultFontCollection();
  auto builder = ParagraphBuilder::make(style, sk_sp<FontCollection>(fontCollection));
  builder->addText(text.c_str());
  auto p = builder->Build();
  p->layout(b.width());
  p->paint(canvas, 0, 0);
}

} // namespace VGG
