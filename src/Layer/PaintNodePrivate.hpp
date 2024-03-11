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
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/Effects.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/DisplayList.hpp"
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Renderer.hpp"
#include "Layer/Core/Transform.hpp"

#include <algorithm>
#include <core/SkBlender.h>
#include <core/SkBlurTypes.h>
#include <core/SkColor.h>
#include <core/SkImage.h>
#include <core/SkMaskFilter.h>
#include <core/SkMatrix.h>
#include <core/SkPaint.h>
#include <core/SkSamplingOptions.h>
#include <core/SkShader.h>
#include <effects/SkImageFilters.h>
#include <encode/SkPngEncoder.h>
#include <core/SkBlendMode.h>
#include <core/SkCanvas.h>
#include <core/SkSurface.h>
#include <core/SkImageFilter.h>
#include <core/SkBBHFactory.h>
#include <core/SkPictureRecorder.h>
#include <effects/SkShaderMaskFilter.h>
#include <effects/SkBlurMaskFilter.h>
#include <src/core/SkBlurMask.h>
#include <src/shaders/SkPictureShader.h>

namespace VGG::layer
{

class MaskObject
{
public:
  struct MaskData
  {
    PaintNode*       mask{ nullptr };
    Transform        transform;
    sk_sp<SkBlender> blender{ nullptr };
    MaskData(PaintNode* p, const Transform& t, sk_sp<SkBlender> blender)
      : mask(p)
      , transform(t)
      , blender(std::move(blender))
    {
    }
  };
  std::vector<MaskData>          components;
  std::optional<sk_sp<SkShader>> shader;

  sk_sp<SkImageFilter> toImagefilter(const SkRect& b, const SkMatrix* mat)
  {
    ensureMaskShader(b, mat);
    return SkImageFilters::Shader(*shader);
  }

  sk_sp<SkImageFilter> maskWith(const SkRect& b, sk_sp<SkImageFilter> input, const SkMatrix* mat)
  {
    return SkImageFilters::Blend(SkBlendMode::kSrcIn, toImagefilter(b, mat), input, b);
  }

  sk_sp<SkMaskFilter> toMaskFilter(const SkRect& b, const SkMatrix* mat)
  {
    ensureMaskShader(b, mat);
    return SkShaderMaskFilter::Make(*shader);
  }

  void ensureMaskShader(const SkRect& b, const SkMatrix* mat)
  {
    if (!shader)
    {
      Renderer          alphaMaskRender;
      SkPictureRecorder rec;
      auto              rt = SkRTreeFactory();
      auto              canvas = rec.beginRecording(b, &rt);
      alphaMaskRender.setCanvas(canvas);
      for (const auto& p : components)
      {
        auto skm = toSkMatrix(p.transform.matrix());
        canvas->save();
        canvas->concat(skm);
        p.mask->onDrawAsAlphaMask(&alphaMaskRender, 0);
        canvas->restore();
      }
      auto maskShader = SkPictureShader::Make(
        rec.finishRecordingAsPicture(),
        SkTileMode::kClamp,
        SkTileMode::kClamp,
        SkFilterMode::kNearest,
        mat,
        &b);
      ASSERT(maskShader);
      shader = maskShader;
    }
  }
};

struct LayerContextGuard
{
public:
  std::optional<SkPaint> paint;
  template<typename F>
  void saveLayer(const ContextSetting& st, F&& f)
  {
    auto bm = toSkBlendMode(st.blendMode);
    if (bm)
    {
      std::visit(
        Overloaded{ [&, this](const sk_sp<SkBlender>& blender)
                    {
                      paint = SkPaint();
                      paint->setBlender(blender);
                    },
                    [&, this](const SkBlendMode& mode)
                    {
                      paint = SkPaint();
                      paint->setBlendMode(mode);
                    } },
        *bm);
    }

    if (st.opacity < 1.0)
    {
      if (!paint)
        paint = SkPaint();
      paint->setAlphaf(st.opacity);
    }
    if (paint)
    {
      f(*paint);
    }
  }
  template<typename F>
  void restore(F&& f)
  {
    if (paint)
    {
      f();
    }
    paint = std::nullopt;
  }
};

class Effect
{
public:
  virtual void render(Renderer* render) = 0;
};

class DropShadowEffect final : public Effect
{
public:
  struct Shadow
  {
    Shadow(sk_sp<SkImageFilter> filter, const DropShadow& prop)
      : filter(std::move(filter))
      , prop(prop)
    {
    }
    sk_sp<SkImageFilter> filter;
    DropShadow           prop;
  };

  void render(Renderer* render) override
  {
  }
  DropShadowEffect(const std::vector<DropShadow>& dropShadow, const SkRect& bounds)

  {
    for (const auto& s : dropShadow)
    {
      if (!s.isEnabled)
        continue;
      auto dropShadowFilter = makeDropShadowImageFilter(
        s,
        Bound{ bounds.x(), bounds.y(), bounds.width(), bounds.height() },
        false,
        0);
      auto r = dropShadowFilter->computeFastBounds(bounds);
      m_imageFilters.emplace_back(dropShadowFilter, s);
      m_bounds.join(r);
    }
  }

  DropShadowEffect& operator=(const DropShadowEffect& other) = delete;
  DropShadowEffect(const DropShadowEffect& other) = delete;
  DropShadowEffect& operator=(DropShadowEffect&& other) = default;
  DropShadowEffect(DropShadowEffect&& other) = default;

  const SkRect& bounds()
  {
    return m_bounds;
  }

  const std::vector<Shadow>& filters() const
  {
    return m_imageFilters;
  }

private:
  std::vector<Shadow> m_imageFilters;
  SkRect              m_bounds;
};

class FillEffect final : public Effect
{
public:
  void render(Renderer* render) override
  {
  }
  FillEffect(const std::vector<Fill>& fills, const SkRect& bounds)
  {
    sk_sp<SkShader> dstShader;
    const auto      bound = Bound{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
    for (const auto& f : fills)
    {
      if (!f.isEnabled)
        continue;
      const auto&     st = f.contextSettings;
      sk_sp<SkShader> srcShader;
      std::visit(
        Overloaded{ [&](const Gradient& g) { srcShader = makeGradientShader(bound, g); },
                    [&](const Color& c) { srcShader = SkShaders::Color(c); },
                    [&](const Pattern& p) { srcShader = makePatternShader(bound, p); } },
        f.type);

      if (st.opacity < 1.0)
      {
        srcShader = srcShader->makeWithColorFilter(
          SkColorFilters::Blend(Color{ 0, 0, 0, st.opacity }, SkBlendMode::kSrcIn));
      }
      if (!dstShader)
      {
        dstShader = srcShader;
      }
      else
      {
        auto bm = toSkBlendMode(f.contextSettings.blendMode);
        if (bm)
        {
          std::visit(
            Overloaded{ [&](const sk_sp<SkBlender>& blender)
                        { dstShader = SkShaders::Blend(blender, dstShader, srcShader); },
                        [&](const SkBlendMode& mode)
                        { dstShader = SkShaders::Blend(mode, dstShader, srcShader); } },
            *bm);
        }
        else
        {
          dstShader = SkShaders::Blend(SkBlendMode::kSrcOver, dstShader, srcShader);
        }
      }
    }
    m_shader = dstShader;
  }

  sk_sp<SkShader> shader() const
  {
    return m_shader;
  }

  const SkRect& bounds()
  {
    return m_bounds;
  }

private:
  SkRect          m_bounds;
  sk_sp<SkShader> m_shader;
};

class InnerShadowEffect : public Effect
{
public:
  void render(Renderer* render) override
  {
  }
  InnerShadowEffect(const std::vector<InnerShadow>& innerShadow, const SkRect& bounds)
  {
    for (const auto& s : innerShadow)
    {
      if (!s.isEnabled)
        continue;
      auto innerShadowFilter = makeInnerShadowImageFilter(
        s,
        Bound{ bounds.x(), bounds.y(), bounds.width(), bounds.height() },
        true,
        false,
        0);
      SkRect r;
      m_imageFilters.emplace_back(innerShadowFilter);
      m_bounds.join(r);
    }
  }

  const SkRect& bounds()
  {
    return m_bounds;
  }

  const std::vector<sk_sp<SkImageFilter>>& filters() const
  {
    return m_imageFilters;
  }

private:
  std::vector<sk_sp<SkImageFilter>> m_imageFilters;
  SkRect                            m_bounds;
};

class PaintNode__pImpl // NOLINT
{
  VGG_DECL_API(PaintNode);

public:
  Bound                    bound;
  Transform                transform;
  std::string              guid{};
  std::vector<std::string> maskedBy{};
  std::vector<AlphaMask>   alphaMaskBy;
  EMaskType                maskType{ MT_NONE };
  EMaskShowType            maskShowType{ MST_INVISIBLE };
  EBoolOp                  clipOperator{ BO_NONE };
  EOverflow                overflow{ OF_HIDDEN };
  EWindingType             windingRule{ WR_EVEN_ODD };
  Style                    style;
  ContextSetting           contextSetting;
  EObjectType              type;
  bool                     visible{ true };

  ContourData                contour;
  PaintOption                paintOption;
  ContourOption              maskOption;
  std::optional<DisplayList> styleDisplayList; // fill + border
  std::optional<VShape>      path;
  std::optional<VShape>      mask;
  std::optional<MaskObject>  alphaMask;

  std::optional<DropShadowEffect>  dropShadowEffects;
  std::optional<InnerShadowEffect> innerShadowEffects;

  LayerContextGuard layerContextGuard;

  sk_sp<SkShader> fillShader;

  PaintNode__pImpl(PaintNode* api, EObjectType type)
    : q_ptr(api)
    , type(type)
  {
  }

  void ensureStyleObjectRecorder(
    const VShape&              shape,
    sk_sp<SkBlender>           blender,
    const std::vector<Fill>&   fills,
    const std::vector<Border>& borders)
  {
    if (!styleDisplayList)
    {
      DisplayListRecorder rec;
      SkRect              r = toSkRect(bound);
      auto                styleBounds = computeStyleBounds(shape, borders, r);

      auto recorder = rec.beginRecording(r, SkMatrix::I());
      q_ptr->onDrawFill(recorder, blender, 0, shape);
      drawBorder(recorder, shape, bound, borders, blender);
      styleDisplayList = rec.finishRecording(styleBounds);
    }
  }

  SkRect computeStyleBounds(
    const VShape&              shape,
    const std::vector<Border>& borders,
    const SkRect&              bound) const
  {
    SkRect rect = bound;
    for (const auto& b : borders)
    {
      if (!b.isEnabled || b.thickness <= 0)
        continue;
      SkPaint strokePen;
      strokePen.setAntiAlias(true);
      populateSkPaint(b, rect, strokePen);
      strokePen.setStrokeJoin(toSkPaintJoin(b.lineJoinStyle));
      strokePen.setStrokeCap(toSkPaintCap(b.lineCapStyle));
      strokePen.setStrokeMiter(b.miterLimit);
      float strokeWidth = b.thickness;
      if (b.position == PP_INSIDE)
      {
        // inside
        strokeWidth = 2.f * b.thickness;
      }
      else if (b.position == PP_OUTSIDE)
      {
        // outside
        strokeWidth = 2.f * b.thickness;
      }
      strokePen.setStrokeWidth(strokeWidth);
      strokePen.setStyle(SkPaint::kStroke_Style);
      if (strokePen.canComputeFastBounds())
      {
        SkRect result;
        strokePen.computeFastBounds(bound, &result);
        rect.join(result);
      }
    }
    return rect;
  }

  void ensureDropShadowEffects(const std::vector<DropShadow>& shadow)
  {
    if (!dropShadowEffects)
    {
      dropShadowEffects = DropShadowEffect(shadow, toSkRect(q_ptr->frameBound()));
    }
  }

  void ensureInnerShadowEffects(const std::vector<InnerShadow>& shadow)
  {
    if (!innerShadowEffects)
    {
      innerShadowEffects = InnerShadowEffect(shadow, toSkRect(q_ptr->frameBound()));
    }
  }

  void ensureFillShader()
  {
    if (fillShader)
      return;
    const auto      bound = q_ptr->frameBound();
    sk_sp<SkShader> dstShader;
    for (const auto& f : style.fills)
    {
      if (!f.isEnabled)
        continue;
      const auto&     st = f.contextSettings;
      sk_sp<SkShader> srcShader;
      std::visit(
        Overloaded{ [&](const Gradient& g) { srcShader = makeGradientShader(bound, g); },
                    [&](const Color& c) { srcShader = SkShaders::Color(c); },
                    [&](const Pattern& p) { srcShader = makePatternShader(bound, p); } },
        f.type);

      if (!dstShader)
      {
        dstShader = srcShader;
      }
      else
      {
        auto bm = toSkBlendMode(f.contextSettings.blendMode);
        if (bm)
        {
          std::visit(
            Overloaded{ [&](const sk_sp<SkBlender>& blender)
                        { dstShader = SkShaders::Blend(blender, dstShader, srcShader); },
                        [&](const SkBlendMode& mode)
                        { dstShader = SkShaders::Blend(mode, dstShader, srcShader); } },
            *bm);
        }
      }
    }
    fillShader = dstShader;
  }

  void ensureAlphaMask(Renderer* renderer)
  {
    if (alphaMask)
      return;
    auto        cache = MaskObject();
    const auto& objects = renderer->maskObjects();
    // auto        maskAreaBound = q_ptr->bound();
    for (auto it = alphaMaskBy.begin(); it != alphaMaskBy.end(); ++it)
    {
      const auto& id = it->id;
      if (id != q_ptr->guid())
      {
        if (auto obj = objects.find(id); obj != objects.end())
        {
          const auto t = obj->second->mapTransform(q_ptr);
          cache.components.emplace_back(obj->second, t, getMaskBlender(it->type));
          // OPTIMIZED:
          // if (auto transformedBound = obj->second->bound().transform(t);
          //     maskAreaBound.isIntersectWith(transformedBound))
          // {
          //   cache.components.emplace_back(obj->second, t, nullptr);
          //   maskAreaBound.intersectWith(transformedBound);
          // }
        }
        else
        {
          DEBUG("No such mask: %s", id.c_str());
        }
      }
    }
    VShape path;
    for (const auto& e : cache.components)
    {
      if (path.isEmpty())
      {
        path = e.mask->asVisualShape(&e.transform);
      }
      else
      {
        path.op(e.mask->asVisualShape(&e.transform), EBoolOp::BO_INTERSECTION);
      }
    }
    if (!path.isEmpty())
    {
      alphaMask = std::move(cache);
    }
  }

  void worldTransform(glm::mat3& mat)
  {
    auto p = q_ptr->parent();
    if (!p)
    {
      mat *= q_ptr->transform().matrix();
      return;
    }
    static_cast<PaintNode*>(p.get())->d_ptr->worldTransform(mat);
    mat *= q_ptr->transform().matrix();
  }

  void drawAsAlphaMaskImpl(Renderer* renderer, sk_sp<SkBlender> blender)
  {
    if (!path)
    {
      path = q_ptr->asVisualShape(0);
      // path = Shape();
    }
    if (path->isEmpty())
    {
      return;
    }
    onDrawStyleImpl(renderer, *path, blender);
  }

  Bound drawBorder(
    Renderer*                  renderer,
    const VShape&              border,
    const Bound&               bound,
    const std::vector<Border>& borders,
    sk_sp<SkBlender>           blender)
  {
    SkRect     resultBounds = border.bounds();
    const auto shapeBounds = resultBounds;
    for (const auto& b : borders)
    {
      if (!b.isEnabled || b.thickness <= 0)
        continue;

      SkPaint strokePen;
      strokePen.setAntiAlias(true);
      strokePen.setBlender(blender);
      // strokePen.setImageFilter(imageFilter);
      populateSkPaint(b, shapeBounds, strokePen);
      bool  inCenter = true;
      float strokeWidth = b.thickness;
      if (b.position == PP_INSIDE && border.isClosed())
      {
        // inside
        strokeWidth = 2.f * b.thickness;
        renderer->canvas()->save();
        border.clip(renderer->canvas(), SkClipOp::kIntersect);
        inCenter = false;
      }
      else if (b.position == PP_OUTSIDE && border.isClosed())
      {
        // outside
        strokeWidth = 2.f * b.thickness;
        renderer->canvas()->save();
        border.clip(renderer->canvas(), SkClipOp::kDifference);
        inCenter = false;
      }
      strokePen.setStrokeWidth(strokeWidth);
      border.draw(renderer->canvas(), strokePen);
      SkRect borderBounds;
      strokePen.computeFastBounds(shapeBounds, &borderBounds);
      resultBounds.join(borderBounds);
      if (!inCenter)
      {
        renderer->canvas()->restore();
      }
    }

    if (false)
    {
      auto                 pt = border.asPath();
      std::vector<SkPoint> pts(pt.countPoints());
      SkPaint              p;
      pt.getPoints(pts.data(), pts.size());
      p.setStrokeWidth(2);
      p.setColor(SK_ColorRED);
      SkFont a;
      renderer->canvas()->drawPoints(SkCanvas::kPoints_PointMode, pts.size(), pts.data(), p);
      for (std::size_t i = 0; i < pts.size(); i++)
      {
        SkPaint textPaint;
        textPaint.setStrokeWidth(0.5);
        textPaint.setColor(SK_ColorBLACK);
        std::string index = std::to_string(i);
        renderer->canvas()->drawSimpleText(
          index.c_str(),
          index.size(),
          SkTextEncoding::kUTF8,
          pts[i].x(),
          pts[i].y(),
          a,
          textPaint);
      }
    }
    return Bound{ resultBounds.x(), resultBounds.y(), resultBounds.width(), resultBounds.height() };
  }

  sk_sp<SkImageFilter> blurImageFilter()
  {
    sk_sp<SkImageFilter> result;
    for (const auto& b : style.blurs)
    {
      if (!b.isEnabled)
        continue;
      sk_sp<SkImageFilter> filter;
      std::visit(
        Overloaded{ [](const BackgroundBlur& blur) {},
                    [&](const LayerBlur& blur) { filter = makeLayerBlurFilter(blur); },
                    [&](const MotionBlur& blur) { filter = makeMotionBlurFilter(blur); },
                    [&, this](const RadialBlur& blur)
                    { filter = makeRadialBlurFilter(blur, q_ptr->frameBound()); } },
        b.type);
      if (result == nullptr)
      {
        result = filter;
      }
      else
      {
        result = SkImageFilters::Compose(result, filter);
      }
    }
    return result;
  }

  sk_sp<SkImageFilter> backgroundBlurImageFilter()
  {
    sk_sp<SkImageFilter> result;
    for (const auto& b : style.blurs)
    {
      if (!b.isEnabled)
        continue;
      if (auto ptr = std::get_if<BackgroundBlur>(&b.type); ptr)
      {
        sk_sp<SkImageFilter> filter;
        filter = makeBackgroundBlurFilter(*ptr);
        if (result == nullptr)
        {
          result = filter;
        }
        else
        {
          result = SkImageFilters::Compose(result, filter);
        }
      }
    }
    return result;
  }

  void beginLayer(
    Renderer*            renderer,
    const SkPaint*       paint,
    const VShape*        clipShape,
    sk_sp<SkImageFilter> backdropFilter)
  {
    renderer->canvas()->save();
    if (clipShape)
    {
      clipShape->clip(renderer->canvas(), SkClipOp::kIntersect);
    }
    auto layerBound = clipShape->bounds();
    renderer->canvas()->saveLayer(
      SkCanvas::SaveLayerRec(&layerBound, paint, backdropFilter.get(), 0));
  }

  void endLayer(Renderer* renderer)
  {
    renderer->canvas()->restore();
    renderer->canvas()->restore();
  }

  void onDrawStyleImpl(Renderer* renderer, const VShape& skPath, sk_sp<SkBlender> blender)
  {
    auto filled = false;
    for (const auto& f : style.fills)
    {
      if (f.isEnabled)
      {
        filled = true;
        break;
      }
    }
    auto  border = false;
    float maxWidth = 0;
    for (const auto& b : style.borders)
    {
      if (b.isEnabled)
      {
        border = true;
        switch (b.position)
        {
          case PP_INSIDE:
            break;
          case PP_CENTER:
            maxWidth = std::max(b.thickness, maxWidth);
            break;
          case PP_OUTSIDE:
            maxWidth = std::max(b.thickness * 2, maxWidth);
            break;
        }
      }
    }
    ensureStyleObjectRecorder(skPath, blender, style.fills, style.borders);
    ensureDropShadowEffects(style.dropShadow);
    for (const auto& f : dropShadowEffects->filters())
    {
      LayerContextGuard g;
      g.saveLayer(
        f.prop.contextSettings,
        [&](const SkPaint& paint) { renderer->canvas()->saveLayer(nullptr, &paint); });
      if (f.prop.clipShadow)
      {
        renderer->canvas()->save();
        skPath.clip(renderer->canvas(), SkClipOp::kDifference);
      }
      if (filled)
      {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kFill_Style);
        if (auto ss = skPath.outset(f.prop.spread, f.prop.spread);
            f.prop.spread != 0.f && ss && !ss->isEmpty())
        {
          auto dropShadowFilter = f.filter;
          p.setImageFilter(f.filter);
          ss->draw(renderer->canvas(), p);
        }
        else
        {
          p.setImageFilter(f.filter);
          p.setAntiAlias(true);
          renderer->canvas()->drawPicture(styleDisplayList->picture(), 0, &p);
        }
      }
      if (border)
      {
        // p.setStyle(SkPaint::kStroke_Style);
        // p.setStrokeWidth(maxWidth);
        // painter.canvas()->drawPath(*path, p);
      }
      if (f.prop.clipShadow)
      {
        renderer->canvas()->restore();
      }
      g.restore([&]() { renderer->canvas()->restore(); });
    }

    styleDisplayList->playback(renderer);

    // SkPaint p;
    // p.setShader(styleDisplayList->asShader());
    // p.setAntiAlias(true);
    // renderer->canvas()->drawRect(r, p);

    if (filled)
    {
      ensureInnerShadowEffects(style.innerShadow);
      for (auto& filter : innerShadowEffects->filters())
      {
        SkPaint p;
        p.setImageFilter(filter);
        p.setAntiAlias(true);
        //  painter.canvas()->drawPaint(p);
        skPath.draw(renderer->canvas(), p);
      }
    }
  }
};
} // namespace VGG::layer
