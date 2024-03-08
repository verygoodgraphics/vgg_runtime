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
#pragma once
#include "Renderer.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Effects.hpp"

#include <core/SkBlendMode.h>
#include <core/SkBlender.h>
#include <core/SkImageFilter.h>
#include <core/SkMaskFilter.h>
#include <include/core/SkClipOp.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPathTypes.h>
#include <include/core/SkPathEffect.h>
#include <include/effects/SkImageFilters.h>
#include <include/core/SkShader.h>
#include <include/core/SkTypes.h>
#include <include/effects/SkRuntimeEffect.h>
#include <include/core/SkCanvas.h>
#include <pathops/SkPathOps.h>
#include <src/core/SkBlurMask.h>

using namespace VGG;
class Painter
{
public:
  enum class EStyle
  {
    FILL,
    STROKE,
    FILL_AND_STROKE
  };

private:
  bool      m_antiAlias{ true };
  Renderer* m_renderer{ nullptr };

  static SkPaint::Style toSkPaintStyle(EStyle style)
  {
    switch (style)
    {
      case EStyle::FILL:
        return SkPaint::kFill_Style;
      case EStyle::STROKE:
        return SkPaint::kStroke_Style;
      case EStyle::FILL_AND_STROKE:
        return SkPaint::kStrokeAndFill_Style;
      default:
        return SkPaint::kStroke_Style;
    };
    return SkPaint::kStroke_Style;
  }

public:
  Painter(Renderer* renderer)
    : m_renderer(renderer)
  {
  }

  Painter(
    Renderer*            renderer,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkMaskFilter>  maskFilter,
    sk_sp<SkBlender>     blender)
    : m_renderer(renderer)
  {
  }

  SkCanvas* canvas()
  {
    return m_renderer->canvas();
  }

  Renderer* renderer()
  {
    return m_renderer;
  }

  [[deprecated]] void drawFill(
    const VShape&        skPath,
    const Bound&         bound,
    const Fill&          f,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender,
    sk_sp<SkMaskFilter>  mask);

  [[deprecated]] void drawPathBorder(
    const VShape&        skPath,
    const Bound&         bound,
    const Border&        b,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender);
};
