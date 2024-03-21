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
#include "Guard.hpp"
#include "ShadowEffects.hpp"
#include "FillEffects.hpp"
#include "Mask.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/Effects.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/ObjectShader.hpp"
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/PaintNode.hpp"
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
#include <numeric>
#include <src/core/SkBlurMask.h>
#include <src/shaders/SkPictureShader.h>

namespace VGG::layer
{

namespace internal_draw
{
inline SkRect drawBorder(
  Renderer*                  renderer,
  const VShape&              border,
  const SkRect&              bounds,
  const std::vector<Border>& borders,
  sk_sp<SkBlender>           blender)
{
  SkRect     resultBounds = bounds;
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
  return resultBounds;
}
} // namespace internal_draw

class PaintNode__pImpl // NOLINT
{
  VGG_DECL_API(PaintNode);

public:
  Bound       bound;
  Transform   transform;
  std::string guid{};

  std::vector<std::string> maskedBy;
  std::vector<AlphaMask>   alphaMaskBy;

  EMaskType      maskType{ MT_NONE };
  EMaskShowType  maskShowType{ MST_INVISIBLE };
  EBoolOp        clipOperator{ BO_NONE };
  EOverflow      overflow{ OF_HIDDEN };
  EWindingType   windingRule{ WR_EVEN_ODD };
  Style          style;
  ContextSetting contextSetting;
  EObjectType    type;
  bool           visible{ true };

  ContourData                 contour;
  PaintOption                 paintOption;
  ContourOption               maskOption;
  std::optional<ObjectShader> styleDisplayList; // fill + border
  std::optional<VShape>       path;

  std::optional<DropShadowEffect>  dropShadowEffects;
  std::optional<InnerShadowEffect> innerShadowEffects;

  PaintNode__pImpl(PaintNode* api, EObjectType type)
    : q_ptr(api)
    , type(type)
  {
  }

  void ensureStyleObjectRecorder(
    const VShape&              shape,
    const VShape&              mask,
    sk_sp<SkBlender>           blender,
    const std::vector<Fill>&   fills,
    const std::vector<Border>& borders)
  {
    if (!styleDisplayList)
    {
      ObjectRecorder rec;
      // SkRect              r = computeStyleBounds(shape, borders, shape.bounds());
      SkRect         styleBounds = toSkRect(bound);
      auto           recorder = rec.beginRecording(styleBounds, SkMatrix::I());

      const auto fillBounds = toSkRect(q_ptr->onDrawFill(recorder, blender, 0, shape, mask));
      const auto borderBounds =
        internal_draw::drawBorder(recorder, shape, shape.bounds(), borders, blender);
      styleBounds.join(fillBounds);
      styleBounds.join(borderBounds);
      auto mat = SkMatrix::Translate(styleBounds.x(), styleBounds.y());
      styleDisplayList = rec.finishRecording(styleBounds, &mat);
    }
  }

  void ensureDropShadowEffects(const std::vector<DropShadow>& shadow, const VShape& shape)
  {
    if (!dropShadowEffects)
    {
      dropShadowEffects = DropShadowEffect(shadow, toSkRect(bound), shape.outset(0, 0).has_value());
    }
  }

  void ensureInnerShadowEffects(const std::vector<InnerShadow>& shadow)
  {
    if (!innerShadowEffects)
    {
      innerShadowEffects = InnerShadowEffect(shadow, toSkRect(q_ptr->frameBound()));
    }
  }

  VShape ensureOutlineMask(Renderer* renderer, EBoolOp maskOp)
  {
    VShape result;
    if (maskedBy.empty())
      return result;
    const auto& objects = renderer->maskObjects();
    for (const auto& id : maskedBy)
    {
      if (id != q_ptr->guid())
      {
        if (auto obj = objects.find(id); obj != objects.end())
        {
          const auto t = obj->second->mapTransform(q_ptr);
          auto       m = obj->second->asVisualShape(&t);
          if (result.isEmpty())
          {
            result = m;
          }
          else
          {
            result.op(m, maskOp);
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
    onDrawStyleImpl(renderer, *path, VShape(), blender);
  }

  sk_sp<SkImageFilter> blurImageFilter()
  {
    sk_sp<SkImageFilter> result;
    for (const auto& b : style.layerEffects)
    {
      if (!b.isEnabled)
        continue;
      sk_sp<SkImageFilter> filter;
      std::visit(
        Overloaded{ [&](const GaussianBlur& blur) { filter = makeLayerBlurFilter(blur); },
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
    for (const auto& b : style.backgroundEffects)
    {
      if (!b.isEnabled)
        continue;
      sk_sp<SkImageFilter> filter;
      filter = makeBackgroundBlurFilter(b.blur);
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

  void onRevalidateImpl()
  {
  }

  void onDrawStyleImpl(
    Renderer*        renderer,
    const VShape&    skPath,
    const VShape&    mask,
    sk_sp<SkBlender> blender)
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
    ensureStyleObjectRecorder(skPath, mask, blender, style.fills, style.borders);
    if (filled)
    {
      ensureDropShadowEffects(style.dropShadow, skPath);
      dropShadowEffects->render(renderer, skPath);
    }

    styleDisplayList->render(renderer);

    if (filled)
    {
      ensureInnerShadowEffects(style.innerShadow);
      innerShadowEffects->render(renderer, skPath);
    }
  }
};
} // namespace VGG::layer
