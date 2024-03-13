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
#include "Effects.hpp"
#include "Renderer.hpp"
#include "Guard.hpp"

namespace VGG::layer
{

class DropShadowEffect final : public Effects
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

  void render(Renderer* renderer, const VShape& skPath) override
  {
    for (const auto& f : filters())
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
        // p.setShader(styleDisplayList->asShader());
        // renderer->canvas()->drawRect(styleDisplayList->bounds(), p);
        skPath.draw(renderer->canvas(), p);
      }
      if (f.prop.clipShadow)
      {
        renderer->canvas()->restore();
      }
      g.restore([&]() { renderer->canvas()->restore(); });
    }
  }
  DropShadowEffect(
    const std::vector<DropShadow>& dropShadow,
    const SkRect&                  bounds,
    bool                           overrideSpread)

  {
    bool canBeMerged = true;
    for (const auto& s : dropShadow)
    {
      if (!s.isEnabled)
        continue;
      auto dropShadowFilter = makeDropShadowImageFilter(
        s,
        Bound{ bounds.x(), bounds.y(), bounds.width(), bounds.height() },
        overrideSpread,
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

  sk_sp<SkImageFilter> mergedFilters() const
  {
    return m_mergedFilter;
  }

  bool clipShadow() const
  {
    return true;
  }

private:
  std::vector<Shadow>  m_imageFilters;
  sk_sp<SkImageFilter> m_mergedFilter;
  SkRect               m_bounds;
};

class InnerShadowEffect : public Effects
{
public:
  void render(Renderer* render, const VShape& shape) override
  {
    if (m_mergedFilter)
    {
      SkPaint p;
      p.setImageFilter(m_mergedFilter);
      p.setAntiAlias(true);
      p.setAlphaf(1.0);
      shape.draw(render->canvas(), p);
    }
    // for (auto& filter : innerShadowEffects->filters())
    // {
    //   SkPaint p;
    //   p.setImageFilter(filter);
    //   p.setAntiAlias(true);
    //   p.setAlphaf(0.5);
    //   //  painter.canvas()->drawPaint(p);
    //   skPath.draw(renderer->canvas(), p);
    // }
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
    if (!m_imageFilters.empty())
      m_mergedFilter = SkImageFilters::Merge(m_imageFilters.data(), m_imageFilters.size());
  }

  const SkRect& bounds()
  {
    return m_bounds;
  }

  const std::vector<sk_sp<SkImageFilter>>& filters() const
  {
    return m_imageFilters;
  }

  sk_sp<SkImageFilter> mergedFilter() const
  {
    return m_mergedFilter;
  }

private:
  std::vector<sk_sp<SkImageFilter>> m_imageFilters;
  sk_sp<SkImageFilter>              m_mergedFilter;
  SkRect                            m_bounds;
};
} // namespace VGG::layer
