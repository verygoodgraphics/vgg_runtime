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
  SkPath                contour;
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
  Bound                     bound;
  Transform                 transform;
  std::string               guid{};
  std::vector<std::string>  maskedBy{};
  std::vector<AlphaMask>    alphaMaskBy;
  Mask                      outlineMask;
  EMaskType                 maskType{ MT_NONE };
  EMaskShowType             maskShowType{ MST_INVISIBLE };
  EBoolOp                   clipOperator{ BO_NONE };
  EOverflow                 overflow{ OF_HIDDEN };
  EWindingType              windingRule{ WR_EVEN_ODD };
  Style                     style;
  ContextSetting            contextSetting;
  EObjectType                type;
  bool                      visible{ true };
  ContourPtr                contour;
  PaintOption               paintOption;
  ContourOption             maskOption;
  std::optional<SkPath>     path;
  std::optional<SkPath>     mask;
  std::optional<MaskObject> alphaMask;
  LayerContextGuard         layerContextGuard;

  PaintNode__pImpl(PaintNode* api, EObjectType type)
    : q_ptr(api)
    , type(type)
  {
  }
  PaintNode__pImpl(const PaintNode__pImpl& other)
  {
    this->operator=(other);
  }

  PaintNode__pImpl& operator=(const PaintNode__pImpl& other)
  {
    bound = other.bound;
    transform = other.transform;
    guid = other.guid + "_Copy";
    maskedBy = other.maskedBy;
    outlineMask = other.outlineMask;
    maskType = other.maskType;
    clipOperator = other.clipOperator;
    overflow = other.overflow;
    windingRule = other.windingRule;
    style = other.style;
    contextSetting = other.contextSetting;
    type = other.type;
    visible = other.visible;
    contour = other.contour;
    paintOption = other.paintOption;
    maskOption = other.maskOption;

    path = other.path;
    mask = other.mask;
    alphaMask = other.alphaMask;
    return *this;
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
    SkPath path;
    for (const auto& e : cache.components)
    {
      if (path.isEmpty())
      {
        path = e.mask->asOutlineMask(&e.transform).outlineMask;
      }
      else
      {
        Op(
          path,
          e.mask->asOutlineMask(&e.transform).outlineMask,
          SkPathOp::kIntersect_SkPathOp,
          &path);
      }
    }
    if (!path.isEmpty())
    {
      alphaMask = std::move(cache);
    }
  }

  // template<typename Iter1, typename Iter2, typename F, typename R>
  // std::vector<MaskObject> calcMaskObjects(Renderer* renderer, Iter1 begin, Iter2 end, F&& f)
  // {
  //   // auto                                          canvas = renderer->canvas();
  //   const auto&    objects = renderer->maskObjects();
  //   std::vector<R> cache;
  //   // auto                                          maskAreaBound = Bound::makeInfinite();
  //   // const auto&                                   selfBound = q_ptr->bound();
  //   for (auto it = begin; it != end; ++it)
  //   {
  //     const auto& id = f(*it);
  //     if (id != q_ptr->guid())
  //     {
  //       if (auto obj = objects.find(id); obj != objects.end())
  //       {
  //         const auto t = obj->second->mapTransform(q_ptr);
  //         // const auto transformedBound = obj->second->bound() * t;
  //         cache.emplace_back(obj->second, t);
  //         cache.emplace_back(std::move(F()));
  //         // if (selfBound.isIntersectWith(transformedBound))
  //         // {
  //         //   cache.emplace_back(obj->second, t);
  //         //   maskAreaBound.intersectWith(transformedBound);
  //         // }
  //       }
  //       else
  //       {
  //         DEBUG("No such mask: %s", id.c_str());
  //       }
  //     }
  //   }
  //   return cache;
  // }

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

  sk_sp<SkImage> fetchBackground(SkCanvas* canvas)
  {
    auto b = toSkRect(q_ptr->bound());
    b = canvas->getTotalMatrix().mapRect(b);
    std::cout << "Image scale: " << canvas->getTotalMatrix().getScaleX() << ", " << q_ptr->name()
              << std::endl;
    SkIRect ir = SkIRect::MakeXYWH(b.x(), b.y(), b.width(), b.height());
    if (!ir.isEmpty())
    {
      auto bg = canvas->getSurface()->makeImageSnapshot(ir);
      DEBUG("Image Area: f[%f, %f, %f, %f]", b.x(), b.y(), b.width(), b.height());
      // std::cout << bg->width() << ", " << bg->height() << std::endl;
      // std::ofstream ofs("bg_" + name() + ".png");
      // if (ofs.is_open())
      // {
      //   auto data =
      //   SkPngEncoder::Encode((GrDirectContext*)canvas->getSurface()->recordingContext(),
      //                                    bg.get(),
      //                                    SkPngEncoder::Options());
      //   ofs.write((char*)data->bytes(), data->size());
      // }
      return bg;
    }
    else
    {
      DEBUG("Invalid bg image area:[%f, %f, %f, %f]", b.x(), b.y(), b.width(), b.height());
    }
    return nullptr;
  }

  // sk_sp<SkImageFilter> makeAlphaMaskBy(SkiaRenderer* renderer)
  // {
  //   if (alphaMaskBy.empty())
  //     return nullptr;
  //   auto                                          canvas = renderer->canvas();
  //   const auto&                                   objects = renderer->maskObjects();
  //   std::vector<std::pair<PaintNode*, glm::mat3>> cache;
  //   auto                                          maskAreaBound = Bound::makeInfinite();
  //   const auto&                                   selfBound = bound;
  //   for (int i = 0; i < alphaMaskBy.size(); i++)
  //   {
  //     const auto& mask = alphaMaskBy[i];
  //     if (mask.id != q_ptr->guid())
  //     {
  //       if (auto obj = objects.find(mask.id); obj != objects.end())
  //       {
  //         const auto t = obj->second->mapTransform(q_ptr);
  //         const auto transformedBound = obj->second->bound() * t;
  //         if (selfBound.isIntersectWith(transformedBound))
  //         {
  //           cache.emplace_back(obj->second, t.matrix());
  //           maskAreaBound.intersectWith(transformedBound);
  //         }
  //       }
  //       else
  //       {
  //         DEBUG("No such mask: %s", mask.id.c_str());
  //       }
  //     }
  //   }
  //   if (cache.empty() || !maskAreaBound.valid())
  //     return nullptr;
  //   maskAreaBound.unionWith(selfBound);
  //   const auto [w, h] = std::pair{ maskAreaBound.width(), maskAreaBound.height() };
  //   if (w <= 0 || h <= 0)
  //     return nullptr;
  //   auto       maskSurface = canvas->getSurface()->makeSurface(w, h);
  //   const auto maskCanvas = maskSurface->getCanvas();
  //   // SkPaint pen;
  //   // pen.setColor(SK_ColorRED);
  //   // pen.setStrokeWidth(2);
  //   // pen.setStyle(SkPaint::kStroke_Style);
  //   // maskCanvas->drawRect({ 0, 0, w, h }, pen);
  //   // pen.setColor(SK_ColorBLUE);
  //   // pen.setStyle(SkPaint::kFill_Style);
  //   // maskCanvas->drawCircle(0, 0, 10, pen);
  //   for (const auto& p : cache)
  //   {
  //     auto skm = toSkMatrix(p.second);
  //     maskCanvas->setMatrix(skm);
  //     // pen.setStrokeWidth(5);
  //     // pen.setColor(SK_ColorGREEN);
  //     // maskCanvas->drawCircle(0, 0, 10, pen);
  //     // maskCanvas->drawLine(0, 0, skm.getTranslateX(), -skm.getTranslateY(), pen);
  //     p.first->d_ptr->drawAsAlphaMask(maskCanvas);
  //   }
  //   auto image = maskSurface->makeImageSnapshot();
  //   auto data = SkPngEncoder::Encode(
  //     (GrDirectContext*)maskSurface->recordingContext(),
  //     image.get(),
  //     SkPngEncoder::Options());
  //   // std::ofstream ofs("alphamask.png");
  //   // if (ofs.is_open())
  //   // {
  //   //   ofs.write((char*)data->data(), data->size());
  //   // }
  //   auto filter = SkImageFilters::Image(image, SkSamplingOptions());
  //   auto blend = SkImageFilters::Blend(SkBlendMode::kSrcIn, filter);
  //   // SkMatrix mat;
  //   // mat.postScale(1, -1);
  //   return blend;
  // }

  void drawAsAlphaMaskImpl(Renderer* renderer, sk_sp<SkBlender> blender)
  {
    if (!path)
    {
      path = q_ptr->asOutlineMask(0).outlineMask;
    }
    if (path->isEmpty())
    {
      return;
    }
    Painter painter(renderer);
    drawRawStyleImpl(painter, *path, blender);
  }

  void drawWithAlphaMask(Renderer* renderer, const SkPath& path, const SkPath& outlineMask)
  {
    SkPaint p;
    auto    b = toSkRect(bound);
    p.setBlendMode(SkBlendMode::kSrcOver);
    auto canvas = renderer->canvas();
    canvas->saveLayer(0, 0);
    canvas->clipRect(b);
    canvas->clipPath(path);
    // clip with path
    Painter painter(renderer);
    drawMaskObjectIntoMaskLayer(renderer, nullptr);
    if (!outlineMask.isEmpty())
    {
      painter.beginClip(outlineMask);
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
      painter.endClip();
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

  void drawBlurBgWithAlphaMask(Renderer* renderer, const SkPath& path, const SkPath& outlineMask)
  {
    Painter    painter(renderer);
    const auto blur = style.blurs[0];
    SkPath     res = path;
    if (alphaMask && !alphaMask->contour.isEmpty())
    {
      Op(res, alphaMask->contour, SkPathOp::kIntersect_SkPathOp, &res);
    }
    if (!outlineMask.isEmpty())
    {
      Op(res, outlineMask, SkPathOp::kIntersect_SkPathOp, &res);
    }
    painter.blurBackgroundBegin(blur.radius, blur.radius, bound, &res);

    // draw alpha mask
    drawMaskObjectIntoMaskLayer(renderer, 0);

    // Objects need to be drawn with SrcOver into layer contains blured background
    // and mask them by dstColor.a

    auto blender = getOrCreateBlender("maskedObject", g_styleMaskBlenderShader);
    if (!outlineMask.isEmpty())
    {
      painter.beginClip(outlineMask);
    }
    drawRawStyleImpl(painter, path, blender);
    if (!outlineMask.isEmpty())
    {
      painter.endClip();
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

  void drawBlurContentWithAlphaMask(
    Renderer*     renderer,
    const SkPath& path,
    const SkPath& outlineMask)
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
      painter.beginClip(outlineMask);
    }
    drawRawStyleImpl(painter, path, SkBlender::Mode(SkBlendMode::kSrcOver));
    if (!outlineMask.isEmpty())
    {
      painter.endClip();
    }
    painter.blurContentEnd(); // draw blur content layer into mask layer
    canvas->restore();        // draw masked layer into canvas
  }

  void drawRawStyleImpl(Painter& painter, const SkPath& skPath, sk_sp<SkBlender> blender)
  {
    // const auto globalAlpha = contextSetting.opacity;
    auto filled = false;
    for (const auto& f : style.fills)
    {
      if (f.isEnabled)
      {
        filled = true;
        break;
      }
    }

    // draw outer shadows
    // 1. check out fills
    {
      if (filled)
      {
        // transparent fill clip out the shadow
        painter.beginClip(skPath, SkClipOp::kDifference);
      }
      for (const auto& s : style.shadows) // simplified into one shadow
      {
        if (!s.isEnabled || s.inner)
          continue;
        if (filled)
          painter.drawShadow(skPath, bound, s, SkPaint::kFill_Style, nullptr);

        for (const auto& b : style.borders)
        {
          if (!b.isEnabled)
            continue;
          painter.drawShadow(skPath, bound, s, SkPaint::kStroke_Style, nullptr);
          break;
        }
      }
      if (filled)
      {
        painter.endClip();
      }
    }

    sk_sp<SkImageFilter> innerShadowFilter;
    for (auto it = style.shadowStyle.rbegin(); it != style.shadowStyle.rend(); ++it)
    {
      auto& s = *it;
      std::visit(
        Overloaded{
          [&](const InnerShadowStyle& s)
          { innerShadowFilter = makeInnerShaderImageFilter(s, false, innerShadowFilter); },
          [&](const OuterShadowStyle& s) {},
        },
        s);
    }
    DEBUG("shadow filter: %d", innerShadowFilter ? 1 : 0);
    SkPaint shadow;
    shadow.setImageFilter(innerShadowFilter);
    q_ptr->paintFill(painter.renderer(), blender, innerShadowFilter, skPath);

    for (const auto& b : style.borders)
    {
      if (!b.isEnabled)
        continue;
      painter.drawPathBorder(skPath, bound, b, nullptr, blender);
    }

    // draw inner shadow
    // painter.beginClip(skPath);
    // for (const auto& s : style.shadows)
    // {
    //   if (!s.isEnabled || !s.inner)
    //     continue;
    //   painter.drawInnerShadow(skPath, bound, s, SkPaint::kFill_Style, nullptr);
    // }
    // painter.endClip();
  }

  PaintNode__pImpl(PaintNode__pImpl&&) noexcept = default;
  PaintNode__pImpl& operator=(PaintNode__pImpl&&) noexcept = delete;
};
} // namespace VGG::layer
