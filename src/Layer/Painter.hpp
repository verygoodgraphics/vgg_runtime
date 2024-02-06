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
#include "Layer/VSkiaPrimitive.hpp"
#include "VSkImageFilters.hpp"
#include "VSkia.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Effects.hpp"
#include "Renderer.hpp"

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

#include <stack>

sk_sp<SkShader> getGradientShader(const Gradient& g, const Bound& bound);

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
  bool                 m_antiAlias{ true };
  Renderer*            m_renderer{ nullptr };
  sk_sp<SkImageFilter> m_imageFilter;
  sk_sp<SkBlender>     m_blender;
  sk_sp<SkMaskFilter>  m_maskFilter;

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

  template<typename Primitive>
  void drawFill(const Primitive& primitive, const SkRect& bound, const Fill& f)
  {
    SkPaint fillPen;
    fillPen.setStyle(SkPaint::kFill_Style);
    fillPen.setAntiAlias(m_antiAlias);
    fillPen.setBlender(m_blender);
    fillPen.setImageFilter(m_imageFilter);
    fillPen.setMaskFilter(m_maskFilter);
    populateSkPaint(f.type, f.contextSettings, bound, fillPen);
    primitive.draw(m_renderer->canvas(), fillPen);
  }

  template<typename Primitive>
  void drawBorder(const Primitive& primitive, const SkRect& bound, const Border& b)
  {
    SkPaint strokePen;
    strokePen.setAntiAlias(m_antiAlias);
    strokePen.setBlender(m_blender);
    strokePen.setImageFilter(m_imageFilter);
    populateSkPaint(b, bound, strokePen);
    bool  inCenter = true;
    float strokeWidth = b.thickness;
    if (b.position == PP_INSIDE && primitive.isClosed())
    {
      // inside
      strokeWidth = 2.f * b.thickness;
      m_renderer->canvas()->save();
      primitive.clip(m_renderer->canvas(), SkClipOp::kIntersect);
      inCenter = false;
    }
    else if (b.position == PP_OUTSIDE && primitive.isClosed())
    {
      // outside
      strokeWidth = 2.f * b.thickness;
      m_renderer->canvas()->save();
      primitive.clip(m_renderer->canvas(), SkClipOp::kDifference);
      inCenter = false;
    }
    strokePen.setStrokeWidth(strokeWidth);
    primitive.draw(m_renderer->canvas(), strokePen);
    if (!inCenter)
    {
      m_renderer->canvas()->restore();
    }
  }

  template<typename Primitive>
  void drawInnerShadow(
    const Primitive&        primitive,
    const SkRect&           bound,
    const InnerShadowStyle& s,
    EStyle                  style)
  {

    SkPaint pen;
    auto    sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
    pen.setAntiAlias(m_antiAlias);
    pen.setImageFilter(
      SkMyImageFilters::DropInnerShadowOnly(s.offsetX, s.offsetY, sigma, sigma, s.color, nullptr));
    m_renderer->canvas()->saveLayer(nullptr, &pen);
    if (s.spread > 0)
      m_renderer->canvas()->scale(1.0 / s.spread, 1.0 / s.spread);
    SkPaint fillPen;
    fillPen.setStyle(toSkPaintStyle(style));
    fillPen.setAntiAlias(m_antiAlias);
    primitive.draw(m_renderer->canvas(), fillPen);
    m_renderer->canvas()->restore();
  }

  template<typename Primitive>
  void drawOuterShadow(
    const Primitive         primitive,
    const SkRect&           bound,
    const OuterShadowStyle& s,
    EStyle                  style)
  {
    SkPaint pen;
    pen.setAntiAlias(m_antiAlias);
    auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
    pen.setImageFilter(
      SkImageFilters::DropShadowOnly(s.offsetX, -s.offsetY, sigma, sigma, s.color, nullptr));
    m_renderer->canvas()->saveLayer(nullptr, &pen); // TODO:: test hint rect
    if (s.spread > 0)
      m_renderer->canvas()->scale(1 + s.spread / 100.0, 1 + s.spread / 100.0);
    SkPaint fillPen;
    fillPen.setStyle(toSkPaintStyle(style));
    primitive.draw(m_renderer->canvas(), fillPen);
    m_renderer->canvas()->restore();
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
    , m_imageFilter(std::move(imageFilter))
    , m_blender(std::move(blender))
    , m_maskFilter(std::move(maskFilter))
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

  void blurBackgroundBegin(float radiusX, float radiusY, const Bound& bound, const SkPath* path)
  {
    auto filter = SkImageFilters::Blur(
      SkBlurMask::ConvertRadiusToSigma(radiusX),
      SkBlurMask::ConvertRadiusToSigma(radiusY),
      nullptr);
    auto b = toSkRect(bound);
    m_renderer->canvas()->save();
    if (path)
      m_renderer->canvas()->clipPath(*path);
    m_renderer->canvas()->saveLayer(SkCanvas::SaveLayerRec(&b, nullptr, filter.get(), 0));
  }

  void blurBackgroundEnd()
  {
    m_renderer->canvas()->restore();
    m_renderer->canvas()->restore();
  }
  void blurContentBegin(
    float            radiusX,
    float            radiusY,
    const Bound&     bound,
    const SkPath*    path,
    sk_sp<SkBlender> blender)
  {
    SkPaint pen;
    auto    filter = SkImageFilters::Blur(
      SkBlurMask::ConvertRadiusToSigma(radiusX),
      SkBlurMask::ConvertRadiusToSigma(radiusY),
      nullptr);
    pen.setImageFilter(std::move(filter));
    // pen.setBlendMode(SkBlendMode::kSrcOver);
    auto     bb = bound;
    // bb.extend(radiusX * 1.5);
    auto     b = toSkRect(bb);
    SkMatrix m = SkMatrix::I();
    // m.preScale(1, 1);
    b = m.mapRect(b);
    pen.setBlender(std::move(blender));
    m_renderer->canvas()->save();
    if (path)
      m_renderer->canvas()->clipPath(*path);
    m_renderer->canvas()->saveLayer(&b, &pen);
  }

  void blurContentEnd()
  {
    m_renderer->canvas()->restore();
    m_renderer->canvas()->restore();
  }

  void beginClip(const SkPath& path, SkClipOp clipOp = SkClipOp::kIntersect)
  {
    m_renderer->canvas()->save();
    m_renderer->canvas()->clipPath(path, clipOp);
  }

  void endClip()
  {
    m_renderer->canvas()->restore();
  }

  [[deprecated]] void drawShadow(
    const SkPath&        skPath,
    const Bound&         bound,
    const Shadow&        s,
    SkPaint::Style       style,
    sk_sp<SkImageFilter> imageFilter);

  [[deprecated]] void drawInnerShadow(
    const SkPath&        skPath,
    const Bound&         bound,
    const Shadow&        s,
    SkPaint::Style       style,
    sk_sp<SkImageFilter> imageFilter);

  [[deprecated]] void drawFill(
    const SkPath&        skPath,
    const Bound&         bound,
    const Fill&          f,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender,
    sk_sp<SkMaskFilter>  mask);

  [[deprecated]] void drawPathBorder(
    const SkPath&        skPath,
    const Bound&         bound,
    const Border&        b,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender);

  // Path Drawing
  void drawPathFill(const SkPath& path, const Fill& fill, const Bound* hint)
  {
    if (hint)
      drawFill(Path(path), toSkRect(*hint), fill);
    else
    {
      auto p = Path(path);
      drawFill(p, path.getBounds(), fill);
    }
  }

  void drawPathBorder(const SkPath& path, const Border& border, const Bound* hint)
  {
    if (hint)
      drawBorder(Path(path), toSkRect(*hint), border);
    else
    {
      auto p = Path(path);
      drawBorder(p, path.getBounds(), border);
    }
  }

  void drawPathOuterShadow(
    SkPath                  path,
    const OuterShadowStyle& shadow,
    EStyle                  style,
    const Bound*            hint)
  {
    if (hint)
    {
      drawOuterShadow(Path(path), toSkRect(*hint), shadow, style);
    }
    else
    {
      drawOuterShadow(Path(path), toSkRect(*hint), shadow, style);
    }
  }

  void drawPathInnerShadow(
    SkPath                  path,
    const InnerShadowStyle& shadow,
    EStyle                  style,
    const Bound*            hint)
  {
  }

  // Rect Drawing
  void drawRectFill(const Bound& rect, const Fill& fill, const Bound* hint)
  {
    if (hint)
      drawFill(Rect(rect), toSkRect(*hint), fill);
    else
      drawFill(Rect(rect), toSkRect(rect), fill);
  }

  void drawRectBorder(const Bound& rect, const Border& border, const Bound* hint)
  {
    if (hint)
      drawBorder(Rect(rect), toSkRect(*hint), border);
    else
    {
      drawBorder(Rect(rect), toSkRect(rect), border);
    }
  }

  void drawRectOuterShadow(
    const Bound&            rect,
    const OuterShadowStyle& shadow,
    EStyle                  style,
    const Bound*            hint)
  {
    if (hint)
      drawOuterShadow(Rect(rect), toSkRect(*hint), shadow, style);
    else
      drawOuterShadow(Rect(rect), toSkRect(rect), shadow, style);
  }

  void drawRectInnerShadow(
    const Bound&            rect,
    const InnerShadowStyle& shadow,
    EStyle                  style,
    const Bound*            hint)
  {
    if (hint)
      drawInnerShadow(Rect(rect), toSkRect(*hint), shadow, style);
    else
      drawInnerShadow(Rect(rect), toSkRect(rect), shadow, style);
  }

  // Oval Drawing
  void drawOvalFill(const Bound& oval, const Fill& fill, const Bound* hint)
  {
    if (hint)
      drawFill(Oval(oval), toSkRect(*hint), fill);
    else
      drawFill(Oval(oval), toSkRect(oval), fill);
  }

  void drawOvalBorder(const Bound& oval, const Border& fill, const Bound* hint)
  {
    if (hint)
      drawBorder(Oval(oval), toSkRect(*hint), fill);
    else
      drawBorder(Oval(oval), toSkRect(oval), fill);
  }

  void drawOvalOuterShadow(
    const Bound&            oval,
    const OuterShadowStyle& shadow,
    EStyle                  style,
    const Bound*            hint)
  {
    if (hint)
      drawOuterShadow(Rect(oval), toSkRect(*hint), shadow, style);
    else
      drawOuterShadow(Rect(oval), toSkRect(oval), shadow, style);
  }

  void drawOvalInnerShadow(
    const Bound&            oval,
    const InnerShadowStyle& shadow,
    EStyle                  style,
    const Bound*            hint)
  {

    if (hint)
      drawInnerShadow(Rect(oval), toSkRect(*hint), shadow, style);
    else
      drawInnerShadow(Rect(oval), toSkRect(oval), shadow, style);
  }

  // Polygon Drawing
  void drawPolygonFill()
  {
  }

  void drawImage(
    const Bound&         bound,
    sk_sp<SkShader>      imageShader,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender)
  {
    SkPaint p;
    p.setShader(std::move(imageShader));
    p.setBlender(std::move(blender));
    p.setImageFilter(std::move(imageFilter));
    m_renderer->canvas()->drawPaint(p);
  }
};
