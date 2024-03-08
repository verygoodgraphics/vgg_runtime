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
#include "Layer/DisplayList.hpp"
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Renderer.hpp"
#include "Layer/Core/Transform.hpp"

#include <core/SkBlender.h>
#include <core/SkBlurTypes.h>
#include <core/SkColor.h>
#include <core/SkImage.h>
#include <core/SkMaskFilter.h>
#include <core/SkMatrix.h>
#include <core/SkPaint.h>
#include <core/SkSamplingOptions.h>
#include <encode/SkPngEncoder.h>
#include <core/SkBlendMode.h>
#include <core/SkCanvas.h>
#include <core/SkSurface.h>
#include <core/SkImageFilter.h>
#include <core/SkBBHFactory.h>
#include <core/SkPictureRecorder.h>
#include <effects/SkShaderMaskFilter.h>
#include <effects/SkBlurMaskFilter.h>
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
  LayerContextGuard          layerContextGuard;

  PaintNode__pImpl(PaintNode* api, EObjectType type)
    : q_ptr(api)
    , type(type)
  {
  }

  void ensureDisplayList(Painter& painter, const VShape& shape, sk_sp<SkBlender> blender)
  {
    if (!styleDisplayList)
    {
      DisplayListRecorder rec;
      auto    recorder = rec.beginRecording(toSkRect(q_ptr->frameBound()), SkMatrix::I());
      Painter p(recorder);
      q_ptr->onDrawFill(recorder, blender, 0, shape);
      for (const auto& b : style.borders)
      {
        if (!b.isEnabled || b.thickness <= 0)
          continue;
        p.drawPathBorder(shape, bound, b, 0, blender);
      }
      styleDisplayList = rec.finishRecording();
    }
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
    Painter painter(renderer);
    onDrawStyleImpl(painter, *path, blender);
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

    ensureDisplayList(painter, skPath, blender);
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
          // auto imageFilter = styleDisplayList->asImageFilter();
          // ASSERT(imageFilter);
          // auto output = SkImageFilters::Compose(imageFilter, dropShadowFilter);
          ss->draw(painter.canvas(), p);
        }
        else
        {
          auto dropShadowFilter = makeDropShadowImageFilter(s, q_ptr->frameBound(), false, 0);
          p.setImageFilter(dropShadowFilter);
          p.setAntiAlias(true);
          // p.setShader(styleDisplayList->asShader());
          // painter.canvas()->drawPaint(p);
          painter.canvas()->drawPicture(styleDisplayList->picture(), 0, &p);
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

    styleDisplayList->playback(painter.renderer());
    if (filled)
    {
      for (const auto& s : style.innerShadow)
      {
        if (!s.isEnabled)
          continue;
        auto innerShadowFilter = makeInnerShadowImageFilter(s, q_ptr->frameBound(), true, false, 0);
        SkPaint p;
        p.setImageFilter(innerShadowFilter);
        // p.setAntiAlias(true);
        //  painter.canvas()->drawPaint(p);
        skPath.draw(painter.canvas(), p);
      }
    }
  }
};
} // namespace VGG::layer
