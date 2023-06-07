#ifndef SkMyImageFilters_DEFINED
#define SkMyImageFilters_DEFINED

#include "core/SkColor.h"
#include "core/SkImageFilter.h"
#include "effects/SkImageFilters.h"

class SK_API SkMyImageFilters
{
public:
  using CropRect = SkImageFilters::CropRect;
  static sk_sp<SkImageFilter> DropInnerShadow(SkScalar dx,
                                              SkScalar dy,
                                              SkScalar sigmaX,
                                              SkScalar sigmaY,
                                              SkColor color,
                                              sk_sp<SkImageFilter> input,
                                              const CropRect& cropRect = {});
  static sk_sp<SkImageFilter> DropInnerShadowOnly(SkScalar dx,
                                                  SkScalar dy,
                                                  SkScalar sigmaX,
                                                  SkScalar sigmaY,
                                                  SkColor color,
                                                  sk_sp<SkImageFilter> input,
                                                  const CropRect& cropRect = {});
};

#endif // SkMyImageFilters_DEFINED
