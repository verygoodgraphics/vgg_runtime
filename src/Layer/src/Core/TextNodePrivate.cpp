#include "Core/TextNodePrivate.h"
#include "Core/FontManager.h"
#include "Core/TextNode.h"

#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

namespace VGG
{

using namespace skia::textlayout;
template<typename F>
void runOnUtf8(const char* utf8, size_t bytes, F&& f)
{
  const char* cur = utf8;
  const char* next_char = nullptr;
  int char_count = 0;
  int bytes_count = 0;
  auto next_utf8_char = [](unsigned char* p_begin, int& char_count) -> const char*
  {
    if (*p_begin >> 7 == 0)
    {
      char_count = 1;
      ++p_begin;
    }
    else if (*p_begin >> 5 == 6 && p_begin[1] >> 6 == 2)
    {
      p_begin += 2;
      char_count = 1;
    }
    else if (*p_begin >> 4 == 0x0E && p_begin[1] >> 6 == 2 && p_begin[2] >> 6 == 2)
    {
      p_begin += 3;
      char_count = 1;
    }
    else if (*p_begin >> 3 == 0x1E && p_begin[1] >> 6 == 2 && p_begin[2] >> 6 == 2 &&
             p_begin[3] >> 6 == 2)
    {
      p_begin += 4;
      char_count = 2;
    }
    else
    {
      assert(false && "invalid utf8");
    }
    return (const char*)p_begin;
  };
  while ((next_char = next_utf8_char((unsigned char*)cur, char_count)))
  {
    bytes_count += (next_char - cur);
    if (bytes_count > bytes)
      break;
    f(cur, next_char, char_count);
    cur = next_char;
  }
}

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
  txtStyle.setColor(s.fillColor.value_or(VGGColor{ 0, 0, 0, 1 }));
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
