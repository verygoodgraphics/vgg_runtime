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
#pragma once

#include "Layer/Core/VNode.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "Pattern.hpp"

#include <core/SkPaint.h>
#include <core/SkBlendMode.h>

#include <include/effects/SkDashPathEffect.h>

namespace VGG::layer
{

class PenNode : public VNode
{
public:
  PenNode(VRefCnt* cnt)
    : VNode(cnt)
  {
  }

  VGG_ATTRIBUTE(AntiAlias, bool, m_antiAlias);

  SkBlendMode getBlendMode(SkBlendMode mode) const
  {
    return std::get<SkBlendMode>(m_blend);
  }

  void setBlendMode(SkBlendMode mode)
  {
    if (std::holds_alternative<SkBlendMode>(m_blend) && std::get<SkBlendMode>(m_blend) == mode)
    {
      return;
    }
    m_blend = mode;
    invalidate();
  }

  sk_sp<SkBlender> getBlender() const
  {
    return std::get<sk_sp<SkBlender>>(m_blend);
  }

  void setBlender(sk_sp<SkBlender> blender)
  {
    if (
      std::holds_alternative<sk_sp<SkBlender>>(m_blend) &&
      std::get<sk_sp<SkBlender>>(m_blend) == blender)
    {
      return;
    }
    m_blend = blender;
    invalidate();
  }

  void setColor(SkColor color)
  {
    if (std::holds_alternative<SkColor>(m_brush) && std::get<SkColor>(m_brush) == color)
    {
      return;
    }
    m_brush = color;
    invalidate();
  }

  SkColor getColor() const
  {
    return std::get<SkColor>(m_brush);
  }

  void setShader(sk_sp<SkShader> shader)
  {
    if (
      std::holds_alternative<sk_sp<SkShader>>(m_brush) &&
      std::get<sk_sp<SkShader>>(m_brush) == shader)
    {
      return;
    }
    m_brush = shader;
    invalidate();
  }

  sk_sp<SkShader> getShader() const
  {
    return std::get<sk_sp<SkShader>>(m_brush);
  }

  VGG_ATTRIBUTE(Style, SkPaint::Style, m_style);
  VGG_ATTRIBUTE(StrokeWidth, float, m_strokeWidth);
  VGG_ATTRIBUTE(StrokeMiter, float, m_strokeMiter);
  VGG_ATTRIBUTE(StrokeJoin, SkPaint::Join, m_strokeJoin);
  VGG_ATTRIBUTE(StrokeCap, SkPaint::Cap, m_strokeCap);
  VGG_ATTRIBUTE(Opacity, float, m_opacity);

  SkPaint paint(const Bounds& bounds) const
  {
    SkPaint paint;
    paint.setAntiAlias(m_antiAlias);
    if (std::holds_alternative<SkBlendMode>(m_blend))
    {
      paint.setBlendMode(std::get<SkBlendMode>(m_blend));
    }
    else if (std::holds_alternative<sk_sp<SkBlender>>(m_blend))
    {
      paint.setBlender(std::get<sk_sp<SkBlender>>(m_blend));
    }
    paint.setStyle(m_style);
    paint.setStrokeWidth(m_strokeWidth);
    paint.setStrokeMiter(m_strokeMiter);
    paint.setStrokeJoin(m_strokeJoin);
    paint.setStrokeCap(m_strokeCap);
    onMakePaint(&paint, bounds);
    paint.setAlpha(paint.getAlpha() * m_opacity);
    return paint;
  }

protected:
  virtual void onMakePaint(SkPaint* paint, const Bounds& bounds) const = 0;

private:
  float                                       m_opacity = 1;
  float                                       m_strokeWidth = 1;
  float                                       m_strokeMiter = 4;
  bool                                        m_antiAlias = true;
  std::variant<SkBlendMode, sk_sp<SkBlender>> m_blend{ SkBlendMode::kSrcOver };
  SkPaint::Style                              m_style = SkPaint::kFill_Style;
  SkPaint::Join                               m_strokeJoin = SkPaint::kMiter_Join;
  SkPaint::Cap                                m_strokeCap = SkPaint::kButt_Cap;
  std::variant<SkColor, sk_sp<SkShader>>      m_brush;
};

class Brush : public PenNode
{
public:
  Brush(VRefCnt* cnt, const Fill& fill)
    : PenNode(cnt)
    , m_fill(fill)
  {
    applyFill(fill);
  }

  // DEPRECATED
  VGG_ATTRIBUTE(Fill, Fill, m_fill);

  void applyFill(const Fill& fill);

  VGG_CLASS_MAKE(Brush);

protected:
  void onMakePaint(SkPaint* paint, const Bounds& bounds) const override;

  Bounds onRevalidate() override;

private:
  Fill                                   m_fill; // DEPRECATED
  mutable std::unique_ptr<ShaderPattern> m_pattern;
  mutable int                            m_currentFrame = 0;
};

class BorderBrush : public PenNode // TODO:: Should inherited from Brush
{
public:
  BorderBrush(VRefCnt* cnt, const Border& border)
    : PenNode(cnt)
    , m_border(border)
  {
    applyBorder(border);
  }

  VGG_ATTRIBUTE(Border, Border, m_border); // DEPRECATED

  void applyBorder(const Border& border);

  VGG_CLASS_MAKE(BorderBrush);

protected:
  void   onMakePaint(SkPaint* paint, const Bounds& bounds) const override;
  Bounds onRevalidate() override;

private:
  Border                                 m_border; // DEPRECATED
  mutable std::unique_ptr<ShaderPattern> m_pattern;
  mutable int                            m_currentFrame = 0;
};

} // namespace VGG::layer
