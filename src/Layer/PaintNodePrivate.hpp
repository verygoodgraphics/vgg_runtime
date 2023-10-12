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
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Renderer.hpp"
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
  std::optional<sk_sp<SkImageFilter>> alphaMask;
  std::optional<std::vector<std::pair<PaintNode*, glm::mat3>>> maskObjects;
  std::optional<sk_sp<SkImage>> blurBG;
  sk_sp<SkImageFilter> blurFilter;

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

  std::vector<std::pair<PaintNode*, glm::mat3>> calcMaskObjects(SkiaRenderer* renderer)
  {
    auto canvas = renderer->canvas();
    const auto& objects = renderer->maskObjects();
    std::vector<std::pair<PaintNode*, glm::mat3>> cache;
    auto maskAreaBound = Bound2::makeInfinite();
    const auto& selfBound = q_ptr->getBound();
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
      return {};
    maskAreaBound.unionWith(selfBound);
    const auto [w, h] = std::pair{ maskAreaBound.width(), maskAreaBound.height() };
    if (w <= 0 || h <= 0)
      return {};
    return cache;
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

  PaintNode__pImpl(PaintNode__pImpl&&) noexcept = default;
  PaintNode__pImpl& operator=(PaintNode__pImpl&&) noexcept = default;
};
} // namespace VGG
