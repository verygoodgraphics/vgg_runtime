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
#include "Layer/Core/Shape.hpp"
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
  // SkPath                contour;
  Shape                 contour;
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
  Mask                     outlineMask;
  EMaskType                maskType{ MT_NONE };
  EMaskShowType            maskShowType{ MST_INVISIBLE };
  EBoolOp                  clipOperator{ BO_NONE };
  EOverflow                overflow{ OF_HIDDEN };
  EWindingType             windingRule{ WR_EVEN_ODD };
  Style                    style;
  ContextSetting           contextSetting;
  EObjectType              type;
  bool                     visible{ true };

  // ContourPtr                contour;
  ContourData               contour;
  PaintOption               paintOption;
  ContourOption             maskOption;
  // std::optional<SkPath>     path;
  std::optional<Shape>      path;
  std::optional<Shape>      mask;
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
    Shape path;
    for (const auto& e : cache.components)
    {
      if (path.isEmpty())
      {
        path = e.mask->asOutlineMask(&e.transform);
      }
      else
      {
        path.op(e.mask->asOutlineMask(&e.transform), EBoolOp::BO_INTERSECTION);
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

  template<typename Iter1, typename Iter2, typename F>
  std::optional<SkPath> mapContourFromThis(
    EBoolOp   maskOp,
    Iter1     itr1,
    Iter2     itr2,
    F&&       f,
    Renderer* renderer)
  {
    if (itr1 == itr2)
      return std::nullopt;
    SkPath      result;
    auto        op = toSkPathOp(maskOp);
    const auto& objects = renderer->maskObjects();
    for (auto it = itr1; it != itr2; ++it)
    {
      const auto& id = f(*it);
      if (id != q_ptr->guid())
      {
        if (auto obj = objects.find(id); obj != objects.end())
        {
          const auto t = obj->second->mapTransform(q_ptr);
          auto       m = obj->second->asOutlineMask(&t);
          if (result.isEmpty())
          {
            result = m.outlineMask;
          }
          else
          {
            Op(result, m.outlineMask, op, &result);
          }
        }
        else
        {
          DEBUG("No such mask: %s", id.c_str());
        }
      }
    }
    return result;
  }

  void drawAsAlphaMaskImpl(Renderer* renderer, sk_sp<SkBlender> blender)
  {
    if (!path)
    {
      path = q_ptr->asOutlineMask(0);
      // path = Shape();
    }
    if (path->isEmpty())
    {
      return;
    }
    Painter painter(renderer);
    drawRawStyleImpl(painter, *path, blender);
  }

  void drawWithAlphaMask(Renderer* renderer, const Shape& path, const Shape& outlineMask)
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
    drawRawStyleImpl(painter, path, blender);
    canvas->restore();
    if (!outlineMask.isEmpty())
    {
      painter.canvas()->restore();
      // painter.endClip();
    }
    canvas->restore();
  }

  // return nullopt indicates no blur effects
  std::optional<EBlurType> blurType(Blur& blur)
  {
    auto hasBlur = style.blurs.empty() ? false : style.blurs[0].isEnabled;
    if (hasBlur)
    {
      blur = style.blurs[0];
      return style.blurs[0].blurType;
    }
    return std::nullopt;
  }

  void drawBlurBgWithAlphaMask(Renderer* renderer, const Shape& path, const Shape& outlineMask)
  {
    Painter    painter(renderer);
    const auto blur = style.blurs[0];
    Shape      res = path;
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
    painter.blurBackgroundBegin(blur.radius, blur.radius, bound, &res);

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
    drawRawStyleImpl(painter, path, blender);
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
        p.mask->drawAsAlphaMask(renderer, p.blender);
        canvas->restore();
      }
    }
  }

  void drawBlurContentWithAlphaMask(Renderer* renderer, const Shape& path, const Shape& outlineMask)
  {
    SkPaint    p;
    auto       bb = bound;
    const auto blur = style.blurs[0];
    bb.extend(blur.radius * 2);
    auto b = toSkRect(bb);
    auto canvas = renderer->canvas();
    canvas->saveLayer(0, 0);
    canvas->clipRect(b);
    drawMaskObjectIntoMaskLayer(renderer, 0);
    Painter painter(renderer);
    // the blured layer need to be drawn into the parent layer contains alpha mask
    // SrcIn blend mode is necessary
    auto    blender = SkBlender::Mode(SkBlendMode::kSrcIn);
    painter.blurContentBegin(blur.radius, blur.radius, bound, nullptr, blender);
    if (!outlineMask.isEmpty())
    {
      // painter.beginClip(outlineMask);
      //
      painter.canvas()->save();
      outlineMask.clip(painter.canvas(), SkClipOp::kIntersect);
    }
    drawRawStyleImpl(painter, path, SkBlender::Mode(SkBlendMode::kSrcOver));
    if (!outlineMask.isEmpty())
    {
      // painter.endClip();
      painter.canvas()->restore();
    }
    painter.blurContentEnd(); // draw blur content layer into mask layer
    canvas->restore();        // draw masked layer into canvas
  }

  void drawRawStyleImplLegacy(Painter& painter, const Shape& skPath, sk_sp<SkBlender> blender)
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
      for (const auto& s : style.shadows) // simplified into one shadow
      {
        if (!s.isEnabled || s.inner)
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

    q_ptr->paintFill(painter.renderer(), blender, 0, skPath);
    for (const auto& b : style.borders)
    {
      if (!b.isEnabled)
        continue;
      painter.drawPathBorder(skPath, bound, b, 0, blender);
    }

    painter.canvas()->save();
    skPath.clip(painter.canvas(), SkClipOp::kIntersect);
    for (auto it = style.shadows.begin(); it != style.shadows.end(); ++it)
    {
      auto& s = *it;
      if (!s.isEnabled || !s.inner)
        continue;
      painter.drawInnerShadow(skPath, bound, s, SkPaint::kFill_Style, nullptr);
    }
    painter.canvas()->restore();
  }

  void drawRawStyleImpl2(Painter& painter, const Shape& skPath, sk_sp<SkBlender> blender)
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

    for (auto it = style.dropShadow.rbegin(); it != style.dropShadow.rend(); ++it)
    {
      const auto& s = *it;
      if (!s.isEnabled)
        return;
      LayerContextGuard g;
      g.saveLayer(
        s.contextSettings,
        [&](const SkPaint& paint) { painter.canvas()->saveLayer(nullptr, &paint); });
      if (s.clipShadow)
      {
        // painter.beginClip(skPath, SkClipOp::kDifference);
        painter.canvas()->save();
        skPath.clip(painter.canvas(), SkClipOp::kDifference);
      }
      auto    dropShadowFilter = makeDropShadowImageFilter(s, q_ptr->frameBound(), true, 0);
      SkPaint p;
      p.setAntiAlias(true);
      p.setImageFilter(dropShadowFilter);
      if (filled)
      {
        p.setStyle(SkPaint::kFill_Style);
        // painter.canvas()->drawPath(*path, p);
        path->draw(painter.canvas(), p);
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
    q_ptr->paintFill(painter.renderer(), blender, 0, skPath);
    for (const auto& b : style.borders)
    {
      if (!b.isEnabled)
        continue;
      painter.drawPathBorder(skPath, bound, b, 0, blender);
    }
    if (filled)
    {
      for (auto it = style.innerShadow.begin(); it != style.innerShadow.end(); ++it)
      {
        auto& s = *it;
        if (!s.isEnabled)
          return;
        auto    innerShadowFilter = makeInnerShadowImageFilter(s, q_ptr->frameBound(), true, 0);
        SkPaint p;
        p.setImageFilter(innerShadowFilter);
        p.setAntiAlias(true);
        // painter.canvas()->drawPath(skPath, p);
        skPath.draw(painter.canvas(), p);
      }
    }
  }

  void drawRawStyleImpl(Painter& painter, const Shape& skPath, sk_sp<SkBlender> blender)
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

    for (auto it = style.dropShadow.rbegin(); it != style.dropShadow.rend(); ++it)
    {
      const auto& s = *it;
      if (!s.isEnabled)
        return;
      LayerContextGuard g;
      g.saveLayer(
        s.contextSettings,
        [&](const SkPaint& paint) { painter.canvas()->saveLayer(nullptr, &paint); });
      if (s.clipShadow)
      {
        // painter.beginClip(skPath, SkClipOp::kDifference);
        painter.canvas()->save();
        skPath.clip(painter.canvas(), SkClipOp::kDifference);
      }
      auto    dropShadowFilter = makeDropShadowImageFilter(s, q_ptr->frameBound(), true, 0);
      SkPaint p;
      p.setAntiAlias(true);
      p.setImageFilter(dropShadowFilter);
      if (filled)
      {
        p.setStyle(SkPaint::kFill_Style);
        // painter.canvas()->drawPath(*path, p);
        path->draw(painter.canvas(), p);
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
    q_ptr->paintFill(painter.renderer(), blender, 0, skPath);
    for (const auto& b : style.borders)
    {
      if (!b.isEnabled)
        continue;
      painter.drawPathBorder(skPath, bound, b, 0, blender);
    }
    if (filled)
    {
      for (auto it = style.innerShadow.begin(); it != style.innerShadow.end(); ++it)
      {
        auto& s = *it;
        if (!s.isEnabled)
          return;
        auto    innerShadowFilter = makeInnerShadowImageFilter(s, q_ptr->frameBound(), true, 0);
        SkPaint p;
        p.setImageFilter(innerShadowFilter);
        p.setAntiAlias(true);
        // painter.canvas()->drawPath(skPath, p);
        skPath.draw(painter.canvas(), p);
      }
    }
  }
};
} // namespace VGG::layer
