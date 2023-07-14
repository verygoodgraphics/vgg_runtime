#pragma once

#include <core/SkFontMgr.h>
#include <src/ports/SkFontMgr_custom.h>
class SkFontMgrVGG : public SkFontMgr_Custom
{
protected:
  sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
  sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
};
