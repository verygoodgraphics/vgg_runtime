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
#include "Layer/Core/VType.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/Painter.hpp"
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Renderer.hpp"

#include <core/SkBlender.h>
#include <encode/SkPngEncoder.h>
#include <core/SkBlendMode.h>
#include <core/SkCanvas.h>
#include <core/SkSurface.h>
#include <core/SkImageFilter.h>
namespace VGG
{

template<typename F>
Bound2 calcMaskAreaIntersection(const Bound2& pruneBound, PaintNode* obj, F&& f)
{
  Bound2 bound;
  while (auto m = f())
  {
    const auto t = m->mapTransform(obj);
    const auto transformedBound = m->getBound() * t;
    bound.intersectWith(transformedBound);
    // auto outlineMask = m->asOutlineMask(&t);
  }
  return bound;
}

struct MaskContour
{
  SkPath contour;
};

class PaintNode__pImpl // NOLINT
{
  VGG_DECL_API(PaintNode);

public:
  Bound2 bound;
  glm::mat3 transform{ 1.0 };
  std::string guid{};
  std::vector<std::string> maskedBy{};
  std::vector<AlphaMask> alphaMaskBy;
  Mask outlineMask;
  EMaskType maskType{ MT_None };
  EMaskShowType maskShowType{ MST_Invisible };
  EBoolOp clipOperator{ BO_None };
  EOverflow overflow{ OF_Hidden };
  EWindingType windingRule{ WR_EvenOdd };
  Style style;
  ContextSetting contextSetting;
  ObjectType type;
  bool visible{ true };

  ContourPtr contour;
  PaintOption paintOption;
  ContourOption maskOption;

  std::optional<SkPath> path;
  std::optional<SkPath> mask;

  std::optional<std::vector<std::pair<PaintNode*, glm::mat3>>> alphaMaskObjects;
  std::optional<SkPath> alphaMaskContour;

  PaintNode__pImpl(PaintNode* api, ObjectType type)
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
    return *this;
  }

  template<typename Iter1, typename Iter2, typename F>
  std::vector<std::pair<PaintNode*, glm::mat3>> calcMaskObjects(SkiaRenderer* renderer,
                                                                Iter1 begin,
                                                                Iter2 end,
                                                                F&& f)
  {
    auto canvas = renderer->canvas();
    const auto& objects = renderer->maskObjects();
    std::vector<std::pair<PaintNode*, glm::mat3>> cache;
    auto maskAreaBound = Bound2::makeInfinite();
    const auto& selfBound = q_ptr->getBound();
    for (auto it = begin; it != end; ++it)
    {
      const auto& id = f(*it);
      if (id != q_ptr->guid())
      {
        if (auto obj = objects.find(id); obj != objects.end())
        {
          const auto t = obj->second->mapTransform(q_ptr);
          const auto transformedBound = obj->second->getBound() * t;
          cache.emplace_back(obj->second, t);
          // if (selfBound.isIntersectWith(transformedBound))
          // {
          //   cache.emplace_back(obj->second, t);
          //   maskAreaBound.intersectWith(transformedBound);
          // }
        }
        else
        {
          DEBUG("No such mask: %s", id.c_str());
        }
      }
    }
    return cache;
    // if (cache.empty() || !maskAreaBound.valid())
    //   return {};
    // maskAreaBound.unionWith(selfBound);
    // const auto [w, h] = std::pair{ maskAreaBound.width(), maskAreaBound.height() };
    // if (w <= 0 || h <= 0)
    //   return {};
    // return cache;
  }

  void worldTransform(glm::mat3& mat)
  {
    auto p = q_ptr->parent();
    if (!p)
    {
      mat *= q_ptr->localTransform();
      return;
    }
    static_cast<PaintNode*>(p.get())->d_ptr->worldTransform(mat);
    mat *= q_ptr->localTransform();
  }

  template<typename Iter1, typename Iter2, typename F>
  std::optional<SkPath> mapContourFromThis(EBoolOp maskOp,
                                           Iter1 itr1,
                                           Iter2 itr2,
                                           F&& f,
                                           SkiaRenderer* renderer)
  {
    if (itr1 == itr2)
      return std::nullopt;
    SkPath result;
    auto op = toSkPathOp(maskOp);
    const auto& objects = renderer->maskObjects();
    for (auto it = itr1; it != itr2; ++it)
    {
      const auto& id = f(*it);
      if (id != q_ptr->guid())
      {
        if (auto obj = objects.find(id); obj != objects.end())
        {
          const auto t = obj->second->mapTransform(q_ptr);
          auto m = obj->second->asOutlineMask(&t);
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
    auto b = toSkRect(q_ptr->getBound());
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

  sk_sp<SkImageFilter> makeAlphaMaskBy(SkiaRenderer* renderer)
  {
    if (alphaMaskBy.empty())
      return nullptr;
    auto canvas = renderer->canvas();
    const auto& objects = renderer->maskObjects();
    std::vector<std::pair<PaintNode*, glm::mat3>> cache;
    auto maskAreaBound = Bound2::makeInfinite();
    const auto& selfBound = bound;
    for (int i = 0; i < alphaMaskBy.size(); i++)
    {
      const auto& mask = alphaMaskBy[i];
      if (mask.id != q_ptr->guid())
      {
        if (auto obj = objects.find(mask.id); obj != objects.end())
        {
          const auto t = obj->second->mapTransform(q_ptr);
          const auto transformedBound = obj->second->getBound() * t;
          if (selfBound.isIntersectWith(transformedBound))
          {
            cache.emplace_back(obj->second, t);
            maskAreaBound.intersectWith(transformedBound);
          }
        }
        else
        {
          DEBUG("No such mask: %s", mask.id.c_str());
        }
      }
    }
    if (cache.empty() || !maskAreaBound.valid())
      return nullptr;
    maskAreaBound.unionWith(selfBound);
    const auto [w, h] = std::pair{ maskAreaBound.width(), maskAreaBound.height() };
    if (w <= 0 || h <= 0)
      return nullptr;
    auto maskSurface = canvas->getSurface()->makeSurface(w, h);
    const auto maskCanvas = maskSurface->getCanvas();
    // SkPaint pen;
    // pen.setColor(SK_ColorRED);
    // pen.setStrokeWidth(2);
    // pen.setStyle(SkPaint::kStroke_Style);
    // maskCanvas->drawRect({ 0, 0, w, h }, pen);
    // pen.setColor(SK_ColorBLUE);
    // pen.setStyle(SkPaint::kFill_Style);
    // maskCanvas->drawCircle(0, 0, 10, pen);
    for (const auto& p : cache)
    {
      auto skm = toSkMatrix(p.second);
      skm.postScale(1, -1);
      maskCanvas->setMatrix(skm);
      // pen.setStrokeWidth(5);
      // pen.setColor(SK_ColorGREEN);
      // maskCanvas->drawCircle(0, 0, 10, pen);
      // maskCanvas->drawLine(0, 0, skm.getTranslateX(), -skm.getTranslateY(), pen);
      p.first->d_ptr->drawAlphaMask(maskCanvas);
    }
    auto image = maskSurface->makeImageSnapshot();
    auto data = SkPngEncoder::Encode((GrDirectContext*)maskSurface->recordingContext(),
                                     image.get(),
                                     SkPngEncoder::Options());
    // std::ofstream ofs("alphamask.png");
    // if (ofs.is_open())
    // {
    //   ofs.write((char*)data->data(), data->size());
    // }
    auto filter = SkImageFilters::Image(image, SkSamplingOptions());
    auto blend = SkImageFilters::Blend(SkBlendMode::kSrcIn, filter);
    // SkMatrix mat;
    // mat.postScale(1, -1);
    return blend;
  }

  void drawAlphaMask(SkCanvas* canvas)
  {
    if (!path)
    {
      path = q_ptr->asOutlineMask(0).outlineMask;
    }
    if (path->isEmpty())
    {
      return;
    }

    Painter render(canvas);
    auto result = SkRuntimeEffect::MakeForBlender(SkString(R"(
			vec4 main(vec4 srcColor, vec4 dstColor){
		       return vec4(srcColor.rgb , srcColor.a);
			}
)"))
                    .effect;

    auto blender = result->makeBlender(nullptr);
    // auto blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    for (const auto& f : style.fills)
    {
      if (!f.isEnabled)
        continue;
      render.drawFill(*path, bound, f, contextSetting.Opacity, nullptr, blender, nullptr);
    }
  }

  void drawRawStyle(Painter& painter, const SkPath& skPath, sk_sp<SkBlender> blender)
  {
    const auto globalAlpha = contextSetting.Opacity;
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
        if (!s.is_enabled || s.inner)
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

    q_ptr->paintFill(painter.canvas(), blender, skPath);

    for (const auto& b : style.borders)
    {
      if (!b.isEnabled)
        continue;
      painter.drawPathBorder(skPath, bound, b, globalAlpha, nullptr, blender);
    }

    // draw inner shadow
    painter.beginClip(skPath);
    for (const auto& s : style.shadows)
    {
      if (!s.is_enabled || !s.inner)
        continue;
      painter.drawInnerShadow(skPath, bound, s, SkPaint::kFill_Style, nullptr);
    }
    painter.endClip();
  }

  PaintNode__pImpl(PaintNode__pImpl&&) noexcept = default;
  PaintNode__pImpl& operator=(PaintNode__pImpl&&) noexcept = default;
};
} // namespace VGG
