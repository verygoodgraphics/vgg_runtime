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
#include "Layer/Core/VType.hpp"
#include "Renderer.hpp"
namespace VGG::layer
{

class FillEffect final : public Effects
{
public:
  void render(Renderer* render, const VShape& shape) override
  {
    if (m_shader)
    {
      SkPaint pen;
      pen.setStyle(SkPaint::kFill_Style);
      pen.setShader(m_shader);
      if (m_blender)
      {
        if (auto p = std::get_if<sk_sp<SkBlender>>(&*m_blender); p)
          pen.setBlender(*p);
        else if (auto p = std::get_if<SkBlendMode>(&*m_blender); p)
          pen.setBlendMode(*p);
      }
      pen.setImageFilter(m_imageFilter);
      pen.setAntiAlias(true);
      shape.draw(render->canvas(), pen);
    }
    // for (size_t i = 0; i < style().fills.size(); ++i)
    // {
    //   auto& f = style().fills[i];
    //   if (!f.isEnabled)
    //     continue;
    //   SkPaint fillPen;
    //   fillPen.setStyle(SkPaint::kFill_Style);
    //   fillPen.setAntiAlias(true);
    //   fillPen.setBlender(blender);
    //   fillPen.setImageFilter(imageFilter);
    //   populateSkPaint(f.type, f.contextSettings, toSkRect(frameBound()), fillPen);
    //   path.draw(renderer->canvas(), fillPen);
    // }
  }
  FillEffect(
    const std::vector<Fill>& fills,
    const SkRect&            bounds,
    sk_sp<SkImageFilter>     imagFilter,
    sk_sp<SkBlender>         blender)
  {
    evalShader(fills, bounds);
    m_blender = blender;
    m_imageFilter = imagFilter;
  }

  sk_sp<SkShader> shader() const
  {
    return m_shader;
  }

  const SkRect& bounds()
  {
    return m_bounds;
  }

private:
  SkRect                                                     m_bounds;
  sk_sp<SkShader>                                            m_shader;
  sk_sp<SkImageFilter>                                       m_imageFilter;
  std::optional<std::variant<SkBlendMode, sk_sp<SkBlender>>> m_blender;

  void evalShader(const std::vector<Fill>& fills, const SkRect& bounds)
  {
    const auto      bound = Bound{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
    sk_sp<SkShader> dstShader;
    sk_sp<SkShader> srcShader;
    for (const auto& f : fills)
    {
      if (!f.isEnabled)
        continue;
      const auto& st = f.contextSettings;
      std::visit(
        Overloaded{ [&](const Gradient& g) { srcShader = makeGradientShader(bound, g); },
                    [&](const Color& c) { srcShader = SkShaders::Color(c); },
                    [&](const Pattern& p) { srcShader = makePatternShader(bound, p); } },
        f.type);

      if (st.opacity < 1.0)
      {
        srcShader = srcShader->makeWithColorFilter(
          SkColorFilters::Blend(Color{ 0, 0, 0, st.opacity }, SkBlendMode::kDstIn));
      }
      auto bm = toSkBlendMode(f.contextSettings.blendMode);
      if (!bm)
        DEBUG("no such blend mode %d", f.contextSettings.blendMode);
      if (!dstShader)
      {
        dstShader = srcShader;
        if (!m_blender)
          m_blender = bm;
      }
      else
      {
        if (bm)
        {
          std::visit(
            Overloaded{ [&](const sk_sp<SkBlender>& blender)
                        { dstShader = SkShaders::Blend(blender, dstShader, srcShader); },
                        [&](const SkBlendMode& mode)
                        { dstShader = SkShaders::Blend(mode, dstShader, srcShader); } },
            *bm);
        }
      }
    }
    m_shader = dstShader;
    m_bounds = bounds;
  }
};
} // namespace VGG::layer
