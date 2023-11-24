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
#include "VSkImageFilters.hpp"
#include "VSkia.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"

#include <core/SkBlendMode.h>
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

template<typename K, typename V>
class LRUCache
{
private:
  struct Entry
  {
    Entry(const K& key, V&& value)
      : fKey(key)
      , fValue(std::move(value))
    {
    }

    K fKey;
    V fValue;
  };

public:
  explicit LRUCache()
  {
  }

  LRUCache(const LRUCache&) = delete;
  LRUCache& operator=(const LRUCache&) = delete;

  V* find(const K& key)
  {
    if (auto it = m_map.find(key); it != m_map.end())
    {
      return it->second->fValue;
    }
    return nullptr;
  }

  V* insert(const K& key, V value)
  {
    if (auto it = m_map.find(key); it != m_map.end())
    {
      return nullptr;
    }
    Entry* entry = new Entry(key, std::move(value));
    m_map[key] = entry;
    return entry;
  }

  V* insertOrUpdate(const K& key, V value)
  {
    if (auto it = m_map.find(key); it != m_map.end())
    {
      *(it->second->fValue) = std::move(value);
      return it->second->fValue;
    }
    else
    {
      Entry* entry = new Entry(key, std::move(value));
      m_map[key] = entry;
      return entry;
    }
  }

  int count() const
  {
    return m_map.count();
  }

  void reset()
  {
    m_map.clear();
  }

private:
  void remove(const K& key)
  {
  }
  std::unordered_map<K, V*> m_map;
};

using namespace VGG;
class Painter
{
  bool                           m_antiAlias{ true };
  SkCanvas*                      m_canvas{ nullptr };
  inline static sk_sp<SkBlender> s_maskBlender1;
  inline static sk_sp<SkBlender> s_maskBlender2;

public:
  static sk_sp<SkBlender> getMaskBlender()
  {
    if (!s_maskBlender1)
    {
      auto result = SkRuntimeEffect::MakeForBlender(SkString(R"(
			vec4 main(vec4 srcColor, vec4 dstColor){
		       return vec4(dstColor.rgb, srcColor.a);
			}
)"));

      if (!result.effect)
      {
        DEBUG("Runtime Effect Failed: %s", result.errorText.data());
        return nullptr;
      }
      auto blender = result.effect->makeBlender(nullptr);
      s_maskBlender1 = std::move(blender);
    }
    return s_maskBlender1;
  };

  static sk_sp<SkBlender> getStyleMaskBlender()
  {
    if (s_maskBlender2)
    {
      auto result = SkRuntimeEffect::MakeForBlender(SkString(R"(
			vec4 main(vec4 srcColor, vec4 dstColor){
			 vec4 color = srcColor + dstColor * (1.0 - srcColor.a);
			 return color * dstColor.a;
			}
			)"));
      if (!result.effect)
      {
        DEBUG("Runtime Effect Failed: %s", result.errorText.data());
        return nullptr;
      }
      auto blender = result.effect->makeBlender(nullptr);
      s_maskBlender2 = std::move(blender);
    }
    return s_maskBlender2;
  }

  Painter(SkCanvas* canvas)
    : m_canvas(canvas)
  {
  }

  SkCanvas* canvas()
  {
    return m_canvas;
  }

  void blurBackgroundBegin(float radiusX, float radiusY, const Bound& bound, const SkPath* path)
  {
    auto filter = SkImageFilters::Blur(
      SkBlurMask::ConvertRadiusToSigma(radiusX),
      SkBlurMask::ConvertRadiusToSigma(radiusY),
      nullptr);
    auto b = toSkRect(bound);
    m_canvas->save();
    if (path)
      m_canvas->clipPath(*path);
    m_canvas->saveLayer(SkCanvas::SaveLayerRec(&b, nullptr, filter.get(), 0));
  }

  void blurBackgroundEnd()
  {
    m_canvas->restore();
    m_canvas->restore();
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
    m_canvas->save();
    if (path)
      m_canvas->clipPath(*path);
    m_canvas->saveLayer(&b, &pen);
  }

  void blurContentEnd()
  {
    m_canvas->restore();
    m_canvas->restore();
  }

  void beginClip(const SkPath& path, SkClipOp clipOp = SkClipOp::kIntersect)
  {
    m_canvas->save();
    m_canvas->clipPath(path, clipOp);
  }

  void endClip()
  {
    m_canvas->restore();
  }

  void drawShadow(
    const SkPath&        skPath,
    const Bound&         bound,
    const Shadow&        s,
    SkPaint::Style       style,
    sk_sp<SkImageFilter> imageFilter);

  void drawInnerShadow(
    const SkPath&        skPath,
    const Bound&         bound,
    const Shadow&        s,
    SkPaint::Style       style,
    sk_sp<SkImageFilter> imageFilter);

  void drawFill(
    const SkPath&        skPath,
    const Bound&         bound,
    const Fill&          f,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender,
    sk_sp<SkMaskFilter>  mask);

  void drawPathBorder(
    const SkPath&        skPath,
    const Bound&         bound,
    const Border&        b,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender);

  void drawImage(
    const Bound          bound,
    sk_sp<SkShader>      imageShader,
    sk_sp<SkImageFilter> imageFilter,
    sk_sp<SkBlender>     blender)
  {
    SkPaint p;
    p.setShader(std::move(imageShader));
    p.setBlender(std::move(blender));
    p.setImageFilter(std::move(imageFilter));
    m_canvas->drawPaint(p);
  }
};
