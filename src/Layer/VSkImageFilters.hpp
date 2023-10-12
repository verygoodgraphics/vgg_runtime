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
#ifndef SkMyImageFilters_DEFINED
#define SkMyImageFilters_DEFINED

#include <include/core/SkColor.h>
#include <include/core/SkImageFilter.h>
#include <include/effects/SkImageFilters.h>

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
