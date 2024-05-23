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
    m_blend = mode;
  }

  sk_sp<SkBlender> getBlender() const
  {
    return std::get<sk_sp<SkBlender>>(m_blend);
  }

  void setBlender(sk_sp<SkBlender> blender)
  {
    m_blend = blender;
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
};

class ShaderPenNode : public PenNode
{
public:
  ShaderPenNode(VRefCnt* cnt)
    : PenNode(cnt)
  {
  }

  VGG_ATTRIBUTE(Shader, sk_sp<SkShader>, m_shader);

  VGG_CLASS_MAKE(ShaderPenNode);

protected:
  void onMakePaint(SkPaint* paint, const Bounds& bounds) const override
  {
    paint->setShader(m_shader);
  }

private:
  sk_sp<SkShader> m_shader;
};

class ColorPenNode : public PenNode
{
public:
  ColorPenNode(VRefCnt* cnt)
    : PenNode(cnt)
  {
  }

  VGG_ATTRIBUTE(Color, SkColor, m_color);

  VGG_CLASS_MAKE(ColorPenNode);

protected:
  void onMakePaint(SkPaint* paint, const Bounds& bounds) const override
  {
    paint->setColor(m_color);
  }

private:
  SkColor m_color;
};

class FillPenNode : public PenNode // rename to FillStyle
{
public:
  FillPenNode(VRefCnt* cnt, Fill fill)
    : PenNode(cnt)
    , m_fill(std::move(fill))
  {
  }
  VGG_ATTRIBUTE(Fill, const Fill&, m_fill);
  VGG_CLASS_MAKE(FillPenNode);

protected:
  void onMakePaint(SkPaint* paint, const Bounds& bounds) const override;

  Bounds onRevalidate() override
  {
    return Bounds();
  }

private:
  Fill m_fill;
};

class BorderPenNode : public PenNode // rename to BorderStyle
{
public:
  BorderPenNode(VRefCnt* cnt, Border border)
    : PenNode(cnt)
    , m_border(std::move(border))
  {
  }

  VGG_ATTRIBUTE(Border, const Border&, m_border);
  VGG_CLASS_MAKE(BorderPenNode);

protected:
  void   onMakePaint(SkPaint* paint, const Bounds& bounds) const override;
  Bounds onRevalidate() override
  {
    return Bounds();
  }

private:
  Border m_border;
};

} // namespace VGG::layer
