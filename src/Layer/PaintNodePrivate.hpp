/*Paint
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
#include "StyleItem.hpp"
#include "Renderer.hpp"
#include "Guard.hpp"
#include "ShadowEffects.hpp"
#include "FillEffects.hpp"
#include "Mask.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/LayerCache.h"
#include "Layer/Memory/VAllocator.hpp"
#include "Layer/ObjectAttribute.hpp"
#include "Layer/GraphicItem.hpp"
#include "Layer/ShapeAttribute.hpp"
#include "Layer/TransformAttribute.hpp"
#include "Layer/ShapeItem.hpp"
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

// #define USE_OLD_CODE

namespace VGG::layer
{

namespace internal
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

} // namespace internal

class PaintNode__pImpl // NOLINT
{
  VGG_DECL_API(PaintNode);

  friend class MaskBuilder;

public:
  std::string guid{};

  EMaskType      maskType{ MT_NONE };
  EMaskShowType  maskShowType{ MST_INVISIBLE };
  EBoolOp        clipOperator{ BO_NONE };
  EOverflow      overflow{ OF_HIDDEN };
  EWindingType   windingRule{ WR_EVEN_ODD };
  ContextSetting contextSetting;
  EObjectType    type;
  bool           visible{ true };
  ContourData    contour;
  PaintOption    paintOption;
  ContourOption  maskOption;

  PaintNode::EventHandler paintNodeEventHandler;

  std::optional<VShape> path;
  bool                  renderable{ false };
  Bounds                bound;

  std::array<float, 4> frameRadius{ 0, 0, 0, 0 };
  float                cornerSmooth{ 0 };

  Ref<StyleItem>            renderNode;
  Ref<TransformAttribute>   transformAttr;
  std::unique_ptr<Accessor> accessor;

  PaintNode__pImpl(PaintNode* api, EObjectType type, bool initBase)
    : q_ptr(api)
    , type(type)
  {
    transformAttr = TransformAttribute::Make();
    api->observe(transformAttr);

    if (initBase)
    {
      Ref<ShapeAttribute> shape;
      auto [c, d] = StyleItem::MakeRenderNode(
        nullptr,
        api,
        transformAttr,
        [&](VAllocator* alloc, ObjectAttribute* object) -> Ref<GraphicItem>
        {
          shape = ShapeAttribute::Make(alloc, api);
          auto vectorObject = ShapeItem::Make(alloc, shape, object);
          return vectorObject;
        });
      auto acc = std::make_unique<ShapeItemAttibuteAccessor>(*d, shape.get());
      accessor = std::move(acc);
      renderNode = std::move(c);
    }
    api->observe(renderNode);
  }

  void onRevalidateImpl()
  {
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
};

} // namespace VGG::layer
