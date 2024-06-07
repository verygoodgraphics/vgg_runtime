/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "PenNode.hpp"
#include "Effects.hpp"
#include "Layer/Pattern.hpp"
#include "Layer/GlobalSettings.hpp"

namespace
{
using namespace VGG;
} // namespace
namespace VGG::layer
{

void Brush::applyFill(const Fill& fill)
{
  setStyle(SkPaint::kFill_Style);
  setEnabled(fill.isEnabled);
  setOpacity(fill.contextSettings.opacity);
  auto bm = toSkBlendMode(fill.contextSettings.blendMode);
  if (bm)
  {
    std::visit(
      Overloaded{ [&](const sk_sp<SkBlender>& blender) { setBlender(blender); },
                  [&](const SkBlendMode& mode) { setBlendMode(mode); } },
      *bm);
  }
  setBrush(fill.type);
}

void Brush::onMakePaint(SkPaint* paint, const Bounds& bounds) const
{
  std::visit(
    Overloaded{ [&](const Gradient& g) { paint->setShader(makeGradientShader(bounds, g)); },
                [&](const Color& c) { paint->setColor(c); },
                [&](const Pattern& p)
                {
                  if (!this->m_pattern)
                  {
                    if (auto a = std::get_if<PatternFit>(&p.instance); a)
                    {
                      this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
                    }
                    else if (auto a = std::get_if<PatternFill>(&p.instance); a)
                    {
                      this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
                    }
                    else if (auto a = std::get_if<PatternStretch>(&p.instance); a)
                    {
                      this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
                    }
                    else if (auto a = std::get_if<PatternTile>(&p.instance); a)
                    {
                      this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
                    }
                  }
                } },
    getBrush());
  if (this->m_pattern)
  {
    if (m_pattern->isValid())
    {
      if (m_pattern->frameCount() >= 1)
      {
        paint->setShader(this->m_pattern->shader(m_currentFrame));
        if (isAnimatedPatternEnabled())
        {
          m_currentFrame++;
          m_currentFrame %= m_pattern->frameCount();
          const_cast<Brush*>(this)->update();
        }
      }
    }
  }
}

Bounds Brush::onRevalidate(Revalidation* inv, const glm::mat3& mat)
{
  return Bounds();
}

void BorderBrush::applyBorder(const Border& border)
{
  setStyle(SkPaint::kStroke_Style);
  setEnabled(border.isEnabled);
  setOpacity(border.contextSettings.opacity);
  setDashPattern(border.dashedPattern);
  setDashPatternOffset(border.dashedOffset);
  setPosition(border.position);
  setStrokeJoin(toSkPaintJoin(border.lineJoinStyle));
  setStrokeCap(toSkPaintCap(border.lineCapStyle));
  setStrokeWidth(border.thickness);
  setStrokeMiter(border.miterLimit);
  auto bm = toSkBlendMode(border.contextSettings.blendMode);
  if (bm)
  {
    std::visit(
      Overloaded{ [&](const sk_sp<SkBlender>& blender) { setBlender(blender); },
                  [&](const SkBlendMode& mode) { setBlendMode(mode); } },
      *bm);
  }

  setBrush(border.type);
}

void BorderBrush::onMakePaint(SkPaint* paint, const Bounds& bounds) const

{
  const auto& dashedPattern = getDashPattern();
  paint->setPathEffect(
    SkDashPathEffect::Make(dashedPattern.data(), dashedPattern.size(), getDashPatternOffset()));
  Brush::onMakePaint(paint, bounds);
}

Bounds BorderBrush::onRevalidate(Revalidation* inv, const glm::mat3& mat)
{
  return Bounds();
}

} // namespace VGG::layer
