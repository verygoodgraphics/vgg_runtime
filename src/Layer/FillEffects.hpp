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
    // if (m_shader)
    // {
    //   SkPaint pen;
    //   pen.setStyle(SkPaint::kFill_Style);
    //   pen.setShader(m_shader);
    //   pen.setImageFilter(m_imageFilter);
    //   pen.setAntiAlias(true);
    //   shape.draw(render->canvas(), pen);
    // }
    for (size_t i = 0; i < m_fills.size(); ++i)
    {
      auto& f = m_fills[i];
      if (!f.isEnabled)
        continue;
      SkPaint fillPen;
      fillPen.setStyle(SkPaint::kFill_Style);
      fillPen.setAntiAlias(true);
      // fillPen.setBlender(m_blender);
      fillPen.setImageFilter(m_imageFilter);
      populateSkPaint(f.type, f.contextSettings, bounds(), fillPen);
      shape.draw(render->canvas(), fillPen);
    }
  }
  FillEffect(
    const std::vector<Fill>& fills,
    const SkRect&            bounds,
    sk_sp<SkImageFilter>     imagFilter,
    sk_sp<SkBlender>         blender)
  {
    m_bounds = bounds;
    m_blender = blender;
    m_imageFilter = imagFilter;
    m_fills = fills;
    // m_shader = evalShader(fills, bounds);
  }

  // sk_sp<SkShader> shader() const
  // {
  //   return m_shader;
  // }

  const SkRect& bounds()
  {
    return m_bounds;
  }

private:
  SkRect               m_bounds;
  std::vector<Fill>    m_fills;
  // sk_sp<SkShader>      m_shader;
  sk_sp<SkImageFilter> m_imageFilter;
  sk_sp<SkBlender>     m_blender;

  sk_sp<SkShader> evalShader(const std::vector<Fill>& fills, const SkRect& bounds)
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
        if (bm)
        {
          sk_sp<SkBlender> bl;
          std::visit(
            Overloaded{ [&](const sk_sp<SkBlender>& blender) { bl = blender; },
                        [&](const SkBlendMode& mode)
                        {
                          bl = SkBlender::Mode(mode);
                          DEBUG("first blend mode: %s", SkBlendMode_Name(mode));
                        } },
            *bm);
          static const char*      s_sksl = R"(
          uniform shader s, d;
          uniform blender b;
          half4 main(float2 xy) {
          return b.eval(s.eval(xy), d.eval(xy));
          })";
          auto                    firstShader = SkRuntimeEffect::MakeForShader(SkString(s_sksl));
          static SkRuntimeEffect* s_blendShader = firstShader.effect.release();

          SkRuntimeEffect::ChildPtr children[] = { std::move(dstShader),
                                                   SkRuntimeEffect::ChildPtr(),
                                                   std::move(bl) };

          dstShader = s_blendShader->makeShader(/*uniforms=*/{}, children);
        }
      }
      else
      {
        if (bm)
        {
          std::visit(
            Overloaded{ [&](const sk_sp<SkBlender>& blender)
                        { dstShader = SkShaders::Blend(blender, dstShader, srcShader); },
                        [&](const SkBlendMode& mode)
                        {
                          dstShader = SkShaders::Blend(mode, dstShader, srcShader);
                          DEBUG("first blend mode: %s", SkBlendMode_Name(mode));
                        } },
            *bm);
        }
      }
    }
    return dstShader;
  }
};
} // namespace VGG::layer
