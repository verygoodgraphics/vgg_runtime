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
#include "Layer/AttrSerde.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/Effects.hpp"
#include "Layer/Painter.hpp"
#include "Layer/VSkia.hpp"
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Renderer.hpp"
#include "Layer/Core/Transform.hpp"

#include <core/SkBlender.h>
#include <core/SkColor.h>
#include <core/SkMatrix.h>
#include <core/SkPaint.h>
#include <encode/SkPngEncoder.h>
#include <core/SkBlendMode.h>
#include <core/SkCanvas.h>
#include <core/SkSurface.h>
#include <core/SkImageFilter.h>

namespace VGG::layer
{

template<typename F>
Bound calcMaskAreaIntersection(const Bound& pruneBound, PaintNode* obj, F&& f)
{
  Bound bound;
  while (auto m = f())
  {
    const auto t = m->mapTransform(obj);
    const auto transformedBound = m->getBound() * t;
    bound.intersectWith(transformedBound);
    // auto outlineMask = m->asOutlineMask(&t);
  }
  return bound;
}

struct MaskObject
{
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
  std::vector<MaskData> components;
  VShape                contour;
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

  ContourData               contour;
  PaintOption               paintOption;
  ContourOption             maskOption;
  // std::optional<SkPath>     path;
  std::optional<VShape>     path;
  std::optional<VShape>     mask;
  std::optional<MaskObject> alphaMask;
  LayerContextGuard         layerContextGuard;

  PaintNode__pImpl(PaintNode* api, EObjectType type)
    : q_ptr(api)
    , type(type)
  {
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
        // Op(
        //   path,
        //   e.mask->asOutlineMask(&e.transform),
        //   SkPathOp::kIntersect_SkPathOp,
        //   &path);
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
    Painter painter(renderer);
    onDrawStyleImpl(painter, *path, blender);
  }

  void drawWithAlphaMask(Renderer* renderer, const VShape& path, const VShape& outlineMask)
  {
    SkPaint p;
    auto    b = toSkRect(bound);
    p.setBlendMode(SkBlendMode::kSrcOver);
    auto canvas = renderer->canvas();
    canvas->saveLayer(0, 0);
    canvas->clipRect(b);
    path.clip(canvas, SkClipOp::kIntersect);
    // canvas->clipPath(path);
    //  clip with path
    Painter painter(renderer);
    drawMaskObjectIntoMaskLayer(renderer, nullptr);
    if (!outlineMask.isEmpty())
    {
      // painter.beginClip(outlineMask);
      painter.canvas()->save();
      outlineMask.clip(painter.canvas(), SkClipOp::kIntersect);
    }
    SkPaint objectLayerPaint;
    objectLayerPaint.setBlendMode(SkBlendMode::kSrcIn);
    canvas->saveLayer(0, &objectLayerPaint);
    canvas->clipRect(b);
    auto blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    onDrawStyleImpl(painter, path, blender);
    canvas->restore();
    if (!outlineMask.isEmpty())
    {
      painter.canvas()->restore();
      // painter.endClip();
    }
    canvas->restore();
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
    SkRect   layerRect = toSkRect(q_ptr->frameBound());
    SkMatrix m = SkMatrix::I();
    layerRect = m.mapRect(layerRect);
    renderer->canvas()->save();
    if (clipShape)
    {
      clipShape->clip(renderer->canvas(), SkClipOp::kIntersect);
    }
    renderer->canvas()->saveLayer(
      SkCanvas::SaveLayerRec(&layerRect, paint, backdropFilter.get(), 0));
  }

  void endLayer(Renderer* renderer)
  {
    renderer->canvas()->restore();
    renderer->canvas()->restore();
  }

  void drawBlurBgWithAlphaMask(
    Renderer*            renderer,
    const VShape&        path,
    const VShape&        outlineMask,
    sk_sp<SkImageFilter> bgBlurImageFilter)
  {
    Painter painter(renderer);
    VShape  res = path;
    if (alphaMask && !alphaMask->contour.isEmpty())
    {
      // Op(res, alphaMask->contour, SkPathOp::kIntersect_SkPathOp, &res);
      res.op(alphaMask->contour, EBoolOp::BO_INTERSECTION);
    }
    if (!outlineMask.isEmpty())
    {
      // Op(res, outlineMask, SkPathOp::kIntersect_SkPathOp, &res);
      res.op(outlineMask, EBoolOp::BO_INTERSECTION);
    }

    painter.blurBackgroundBegin(bgBlurImageFilter, bound, &res);

    // draw alpha mask
    drawMaskObjectIntoMaskLayer(renderer, 0);

    // Objects need to be drawn with SrcOver into layer contains blured background
    // and mask them by dstColor.a

    auto blender = getOrCreateBlender("maskedObject", g_styleMaskBlenderShader);
    if (!outlineMask.isEmpty())
    {
      // painter.beginClip(outlineMask);
      painter.canvas()->save();
      outlineMask.clip(painter.canvas(), SkClipOp::kIntersect);
    }
    onDrawStyleImpl(painter, path, blender);
    if (!outlineMask.isEmpty())
    {
      // painter.endClip();
      painter.canvas()->restore();
    }
    painter.blurBackgroundEnd();
  }

  void drawMaskObjectIntoMaskLayer(Renderer* renderer, sk_sp<SkBlender> blender)
  {
    if (alphaMask)
    {
      auto canvas = renderer->canvas();
      for (const auto& p : alphaMask->components)
      {
        auto skm = toSkMatrix(p.transform.matrix());
        canvas->save();
        canvas->concat(skm);
        p.mask->onDrawAsAlphaMask(renderer, p.blender);
        canvas->restore();
      }
    }
  }

  void drawBlurContentWithAlphaMask(
    Renderer*            renderer,
    const VShape&        path,
    const VShape&        outlineMask,
    sk_sp<SkImageFilter> blurImageFilter)
  {
    SkPaint p;
    auto    bb = bound;
    // bb.extend(blur.radius * 2);
    auto    b = toSkRect(bb);
    auto    canvas = renderer->canvas();
    canvas->saveLayer(0, 0);
    canvas->clipRect(b);
    drawMaskObjectIntoMaskLayer(renderer, 0);
    Painter painter(renderer);
    // the blured layer need to be drawn into the parent layer contains alpha mask
    // SrcIn blend mode is necessary
    auto    blender = SkBlender::Mode(SkBlendMode::kSrcIn);
    painter.blurContentBegin(blurImageFilter, bound, nullptr, blender);
    if (!outlineMask.isEmpty())
    {
      // painter.beginClip(outlineMask);
      painter.canvas()->save();
      outlineMask.clip(painter.canvas(), SkClipOp::kIntersect);
    }
    onDrawStyleImpl(painter, path, SkBlender::Mode(SkBlendMode::kSrcOver));
    if (!outlineMask.isEmpty())
    {
      // painter.endClip();
      painter.canvas()->restore();
    }

    painter.blurContentEnd(); // draw blur content layer into mask layer
    canvas->restore();        // draw masked layer into canvas
  }

  void drawRawStyleImplLegacy(Painter& painter, const VShape& skPath, sk_sp<SkBlender> blender)
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
    {
      if (filled)
      {
        // transparent fill clip out the shadow
        // painter.beginClip(skPath, SkClipOp::kDifference);
        painter.canvas()->save();
        skPath.clip(painter.canvas(), SkClipOp::kDifference);
      }
      for (const auto& s : style.dropShadow) // simplified into one shadow
      {
        if (!s.isEnabled)
          continue;
        if (filled)
          painter.drawShadow(skPath, bound, s, SkPaint::kFill_Style, nullptr);

        // for (const auto& b : style.borders)
        // {
        //   if (!b.isEnabled)
        //     continue;
        //   painter.drawShadow(skPath, bound, s, SkPaint::kStroke_Style, nullptr);
        //   break;
        // }
      }
      if (filled)
      {
        painter.canvas()->restore();
      }
    }

    q_ptr->onDrawFill(painter.renderer(), blender, 0, skPath);
    for (const auto& b : style.borders)
    {
      if (!b.isEnabled)
        continue;
      painter.drawPathBorder(skPath, bound, b, 0, blender);
    }

    painter.canvas()->save();
    skPath.clip(painter.canvas(), SkClipOp::kIntersect);
    for (const auto& s : style.innerShadow)
    {
      if (!s.isEnabled)
        continue;
      painter.drawInnerShadow(skPath, bound, s, SkPaint::kFill_Style, nullptr);
    }
    painter.canvas()->restore();
  }

  void onDrawStyleImpl(Painter& painter, const VShape& skPath, sk_sp<SkBlender> blender)
  {
    // return drawRawStyleImplLegacy(painter, skPath, blender);
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

    for (const auto& s : style.dropShadow)
    {
      if (!s.isEnabled)
        continue;
      LayerContextGuard g;
      g.saveLayer(
        s.contextSettings,
        [&](const SkPaint& paint) { painter.canvas()->saveLayer(nullptr, &paint); });
      if (s.clipShadow)
      {
        painter.canvas()->save();
        skPath.clip(painter.canvas(), SkClipOp::kDifference);
      }
      if (filled)
      {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kFill_Style);
        if (auto ss = skPath.outset(s.spread, s.spread); s.spread != 0.f && ss && !ss->isEmpty())
        {
          auto dropShadowFilter = makeDropShadowImageFilter(s, q_ptr->frameBound(), true, 0);
          p.setImageFilter(dropShadowFilter);
          ss->draw(painter.canvas(), p);
        }
        else
        {
          auto dropShadowFilter = makeDropShadowImageFilter(s, q_ptr->frameBound(), false, 0);
          p.setImageFilter(dropShadowFilter);
          skPath.draw(painter.canvas(), p);
        }
      }
      if (border)
      {
        // p.setStyle(SkPaint::kStroke_Style);
        // p.setStrokeWidth(maxWidth);
        // painter.canvas()->drawPath(*path, p);
      }
      if (s.clipShadow)
      {
        painter.canvas()->restore();
      }
      g.restore([&]() { painter.canvas()->restore(); });
    }
    q_ptr->onDrawFill(painter.renderer(), blender, 0, skPath);
    for (const auto& b : style.borders)
    {
      if (!b.isEnabled || b.thickness <= 0)
        continue;
      painter.drawPathBorder(skPath, bound, b, 0, blender);
    }
    if (filled)
    {
      for (const auto& s : style.innerShadow)
      {
        if (!s.isEnabled)
          continue;
        auto innerShadowFilter = makeInnerShadowImageFilter(s, q_ptr->frameBound(), true, false, 0);
        SkPaint p;
        p.setImageFilter(innerShadowFilter);
        p.setAntiAlias(true);
        skPath.draw(painter.canvas(), p);
      }
    }
  }
};
} // namespace VGG::layer
