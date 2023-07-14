#include <core/SkFontMgr.h>
#include <core/SkTypeface.h>
#include <SkiaBackend/SkFontMgrVGG.h>
#include <rapidfuzz/fuzz.hpp>
#include <src/ports/SkFontMgr_custom.h>

// lazy loading
sk_sp<SkTypeface> SkFontMgrVGG::onMakeFromFile(const char path[], int ttcIndex) const
{
  return 0;
}
sk_sp<SkFontStyleSet> SkFontMgrVGG::onMatchFamily(const char familyName[]) const
{
  return SkFontMgr_Custom::onMatchFamily(familyName);
}
