/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <skia/include/core/SkFont.h>
#include <skia/include/core/SkFontTypes.h>
#include <skia/include/core/SkFontMetrics.h>
#include <skia/include/effects/SkImageFilters.h>
#include <skia/include/core/SkTextBlob.h>
#include <skia/src/core/SkBlurMask.h>
#include <skia/custom/SkMyImageFilters.h>

#include <variant>

#include "Entity/InputManager.hpp"
#include "Systems/RenderSystem.hpp"
#include "Systems/GeometrySystem.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Math.hpp"
#include "Utils/FillTypeChooser.hpp"
#include "Utils/TextureManager.hpp"

namespace VGG
{

namespace RenderSystem
{

void drawPathFill(SkCanvas* canvas,
                  const Frame& frame,
                  const SkPath& skPath,
                  const FillStyle* style,
                  double globalAlpha)
{
  ASSERT(canvas);
  SkPaint fillPen;
  fillPen.setStyle(SkPaint::kFill_Style);

  if (!style)
  {
    canvas->drawPath(skPath, fillPen);
  }
  else if (FillTypeChooser<FillStyle>::isFlat(*style))
  {
    fillPen.setColor(style->color);
    fillPen.setAlphaf(fillPen.getAlphaf() * globalAlpha);
    canvas->drawPath(skPath, fillPen);
  }
  else if (FillTypeChooser<FillStyle>::isLinearGradient(*style))
  {
    fillPen.setShader(style->gradient.getLinearShader(frame));
    fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
    canvas->drawPath(skPath, fillPen);
  }
  else if (FillTypeChooser<FillStyle>::isRadialGradient(*style))
  {
    fillPen.setShader(style->gradient.getRadialShader(frame));
    fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
    canvas->drawPath(skPath, fillPen);
  }
  else if (FillTypeChooser<FillStyle>::isAngularGradient(*style))
  {
    fillPen.setShader(style->gradient.getAngularShader(frame));
    fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
    canvas->drawPath(skPath, fillPen);
  }
  else if (style->fillType == FillStyle::FillType::IMAGE)
  {
    if (auto& name = style->imageName)
    {
      fillPen.setShader(style->getImageShader(frame));
      fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
      canvas->drawPath(skPath, fillPen);
    }
  }
}

void drawPathBorder(SkCanvas* canvas,
                    const Frame& frame,
                    const SkPath& skPath,
                    const BorderStyle& style,
                    const BorderOptions& bo,
                    double globalAlpha)
{
  ASSERT(canvas);
  SkPaint strokePen;
  strokePen.setAntiAlias(true);
  strokePen.setStyle(SkPaint::kStroke_Style);
  strokePen.setStrokeWidth(style.thickness);
  strokePen.setPathEffect(bo.getDashEffect());
  strokePen.setStrokeJoin(bo.getSkiaLineJoin());
  strokePen.setStrokeCap(bo.getSkiaLineCap());
  strokePen.setStrokeMiter(bo.miterLimit);

  if (style.position == BorderStyle::BorderPosition::INSIDE)
  {
    strokePen.setStrokeWidth(2. * style.thickness);
    canvas->save();
    canvas->clipPath(skPath, SkClipOp::kIntersect);
  }
  else if (style.position == BorderStyle::BorderPosition::OUTSIDE)
  {
    strokePen.setStrokeWidth(2. * style.thickness);
    canvas->save();
    canvas->clipPath(skPath, SkClipOp::kDifference);
  }

  if (FillTypeChooser<BorderStyle>::isFlat(style))
  {
    strokePen.setColor(style.color);
    strokePen.setAlphaf(strokePen.getAlphaf() * globalAlpha);
    canvas->drawPath(skPath, strokePen);
  }
  else if (FillTypeChooser<BorderStyle>::isLinearGradient(style))
  {
    strokePen.setShader(style.gradient.getLinearShader(frame));
    strokePen.setAlphaf(style.contextSettings.opacity * globalAlpha);
    canvas->drawPath(skPath, strokePen);
  }
  else if (FillTypeChooser<BorderStyle>::isRadialGradient(style))
  {
    strokePen.setShader(style.gradient.getRadialShader(frame));
    strokePen.setAlphaf(style.contextSettings.opacity * globalAlpha);
    canvas->drawPath(skPath, strokePen);
  }
  else if (FillTypeChooser<BorderStyle>::isAngularGradient(style))
  {
    strokePen.setShader(style.gradient.getAngularShader(frame));
    strokePen.setAlphaf(style.contextSettings.opacity * globalAlpha);
    canvas->drawPath(skPath, strokePen);
  }

  if (style.position == BorderStyle::BorderPosition::INSIDE ||
      style.position == BorderStyle::BorderPosition::OUTSIDE)
  {
    canvas->restore();
  }
}

void drawFramedPath(SkCanvas* canvas, const FramedPath& fp, RenderCase rc)
{
  ASSERT(canvas);
  auto& frame = fp.frame;
  ASSERT(frame.w > 0);
  ASSERT(frame.h > 0);

  if (fp.path.points.size() < 2)
  {
    return;
  }

  canvas->save();
  canvas->translate(frame.x, frame.y);
  canvas->translate(frame.w / 2, frame.h / 2);
  canvas->scale(frame.flipX ? -1 : 1, frame.flipY ? -1 : 1);
  canvas->rotate(frame.rotation);
  canvas->translate(-frame.w / 2, -frame.h / 2);

  SkPath skPath = GeometrySystem::getSkiaPath(fp);

  auto& bo = fp.style.borderOptions;
  if (rc == RenderCase::RC_HOVER)
  {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(SK_ColorBLUE);
    p.setStrokeWidth(2);
    p.setStrokeJoin(bo.getSkiaLineJoin());
    p.setStrokeCap(bo.getSkiaLineCap());
    p.setStrokeMiter(bo.miterLimit);
    canvas->drawPath(skPath, p);
  }
  else if (rc == RenderCase::RC_EDIT)
  {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(SK_ColorLTGRAY);
    p.setStrokeJoin(bo.getSkiaLineJoin());
    p.setStrokeCap(bo.getSkiaLineCap());
    p.setStrokeMiter(bo.miterLimit);
    canvas->drawPath(skPath, p);
  }
  else // RC_DEFAULT
  {
    if (fp.style.windingRule == PathStyle::WindingRule::EVENODD)
    {
      skPath.setFillType(SkPathFillType::kEvenOdd);
    }
    else
    {
      skPath.setFillType(SkPathFillType::kWinding);
    }

    // global blend
    auto globalBM = fp.style.contextSettings.blendMode;
    if (globalBM != SkBlendMode::kSrcOver)
    {
      // TODO proper global blend with correct bounds
      SkPaint globalBlendPaint;
      globalBlendPaint.setBlendMode(globalBM);
      canvas->saveLayer(nullptr, &globalBlendPaint);
    }

    // draw blur
    if (fp.style.blur.isEnabled)
    {
      SkPaint pen;
      auto& bs = fp.style.blur.style;
      auto sigma = SkBlurMask::ConvertRadiusToSigma(bs.radius);
      pen.setImageFilter(SkImageFilters::Blur(sigma, sigma, nullptr));
      canvas->saveLayer(nullptr, &pen);
    }

    auto globalAlpha = fp.style.contextSettings.opacity;

    // draw shadows
    if (fp.style.isFilled())
    {
      canvas->save();
      canvas->clipPath(skPath, SkClipOp::kDifference);
    }
    for (auto& shadow : fp.style.shadows)
    {
      if (!shadow.isEnabled)
      {
        continue;
      }
      auto& s = shadow.style;
      SkPaint pen;
      auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blurRadius);
      pen.setImageFilter(
        SkImageFilters::DropShadowOnly(s.offset.x, s.offset.y, sigma, sigma, s.color, nullptr));
      canvas->saveLayer(nullptr, &pen);

      canvas->translate(frame.w / 2 + s.offset.x, frame.h / 2 + s.offset.y);
      canvas->scale(s.spreadRatio, s.spreadRatio);
      canvas->translate(-frame.w / 2 - s.offset.x, -frame.h / 2 - s.offset.y);

      // draw fills for shadow
      if (fp.style.isFilled())
      {
        drawPathFill(canvas, frame, skPath, nullptr, globalAlpha);
      }

      // draw borders for shadow
      for (auto& bs : fp.style.borders)
      {
        if (!bs.isEnabled)
        {
          continue;
        }
        drawPathBorder(canvas, frame, skPath, bs.style, bo, globalAlpha);
      }

      // special case
      if (!(fp.style.isFilled()) && !(fp.style.isStroked()))
      {
        drawPathFill(canvas, frame, skPath, nullptr, globalAlpha);
      }

      canvas->restore();
    }
    if (fp.style.isFilled())
    {
      canvas->restore();
    }

    // draw actual fills
    SkRect bounds = frame.toLocalSkRect();
    for (auto& fs : fp.style.fills)
    {
      if (!fs.isEnabled)
      {
        continue;
      }

      if (auto bm = fs.style.contextSettings.blendMode; bm != SkBlendMode::kSrcOver)
      {
        SkPaint blendPaint;
        blendPaint.setBlendMode(bm);
        canvas->saveLayer(&bounds, &blendPaint);

        drawPathFill(canvas, frame, skPath, &(fs.style), globalAlpha);

        canvas->restore();
      }
      else
      {
        drawPathFill(canvas, frame, skPath, &(fs.style), globalAlpha);
      }
    }

    // draw actual borders
    for (auto& bs : fp.style.borders)
    {
      if (!bs.isEnabled)
      {
        continue;
      }

      if (auto bm = bs.style.contextSettings.blendMode; bm != SkBlendMode::kSrcOver)
      {
        SkPaint blendPaint;
        blendPaint.setBlendMode(bm);
        canvas->saveLayer(nullptr, &blendPaint);

        drawPathBorder(canvas, frame, skPath, bs.style, bo, globalAlpha);

        canvas->restore();
      }
      else
      {
        drawPathBorder(canvas, frame, skPath, bs.style, bo, globalAlpha);
      }
    }

    // draw inner shadows
    canvas->save();
    canvas->clipPath(skPath, SkClipOp::kIntersect);
    for (auto& is : fp.style.innerShadows)
    {
      if (!is.isEnabled)
      {
        continue;
      }
      auto& s = is.style;
      SkPaint pen;
      auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blurRadius);
      pen.setImageFilter(SkMyImageFilters::DropInnerShadowOnly(s.offset.x,
                                                               s.offset.y,
                                                               sigma,
                                                               sigma,
                                                               s.color,
                                                               nullptr));
      canvas->saveLayer(nullptr, &pen);

      canvas->translate(frame.w / 2 + s.offset.x, frame.h / 2 + s.offset.y);
      ASSERT(std::fabs(s.spreadRatio) > FZERO);
      canvas->scale(1. / s.spreadRatio, 1. / s.spreadRatio);
      canvas->translate(-frame.w / 2 - s.offset.x, -frame.h / 2 - s.offset.y);

      // draw fills for inner shadow
      drawPathFill(canvas, frame, skPath, nullptr, globalAlpha);

      canvas->restore();
    }
    canvas->restore();

    // restore for blur
    if (fp.style.blur.isEnabled)
    {
      canvas->restore();
    }

    // restore for global blend
    if (globalBM != SkBlendMode::kSrcOver)
    {
      canvas->restore();
    }
  }

  canvas->restore();
}

void drawFramedText(SkCanvas* canvas, const FramedText& ft)
{
  ASSERT(canvas);
  auto& frame = ft.frame;
  ASSERT(frame.w > 0);
  ASSERT(frame.h > 0);
  auto& textStyle = ft.style;

  canvas->save();
  canvas->translate(frame.x, frame.y);
  canvas->translate(frame.w / 2, frame.h / 2);
  canvas->scale(frame.flipX ? -1 : 1, frame.flipY ? -1 : 1);
  canvas->rotate(frame.rotation);
  canvas->translate(-frame.w / 2, -frame.h / 2);
  canvas->clipRect(frame.toLocalSkRect());

  SkFontMetrics metrics;
  SkFont font = textStyle.getFont();
  font.getMetrics(&metrics);
  double y = std::fabs(metrics.fAscent);
  double df = std::fabs(metrics.fAscent) + std::fabs(metrics.fDescent);
  double dh = textStyle.lineHeightRatio * df;
  double dl = textStyle.paraSpacing;

  if (textStyle.vertAlignment == TextStyle::VertAlignment::VA_CENTER)
  {
    double totalHeight = frame.h;
    auto sz = ft.getLines().size();
    double textHeight = std::max(0.0, sz * dh + sz * dl - dl);
    double dy = (totalHeight - textHeight) / 2;
    y += dy;
  }
  else if (textStyle.vertAlignment == TextStyle::VertAlignment::VA_BOTTOM)
  {
    double totalHeight = frame.h;
    auto sz = ft.getLines().size();
    double textHeight = std::max(0.0, sz * dh + sz * dl - dl);
    double dy = totalHeight - textHeight;
    y += dy;
  }

  SkPaint pen = textStyle.getPaint();
  for (auto line : ft.getLines())
  {
    if (line.empty())
    {
      y += (dh + dl);
      continue;
    }

    double tw = 0;
    std::vector<SkScalar> xs;
    for (size_t i = 0; i < line.size(); i++)
    {
      xs.push_back(tw);
      auto c = TextCodecs::conv.to_bytes(line[i]);
      auto cw = font.measureText(c.c_str(), c.size(), SkTextEncoding::kUTF8);
      tw += (cw + textStyle.letterSpacing);
    }
    tw -= textStyle.letterSpacing;

    auto s = TextCodecs::conv.to_bytes(line);
    auto tb = SkTextBlob::MakeFromPosTextH(s.data(),
                                           s.size(),
                                           xs.data(),
                                           (dh - df) / 2,
                                           font,
                                           SkTextEncoding::kUTF8);

    double x = 0;
    if (textStyle.horzAlignment == TextStyle::HorzAlignment::HA_CENTER)
    {
      x += (frame.w - tw) / 2;
    }
    else if (textStyle.horzAlignment == TextStyle::HorzAlignment::HA_RIGHT)
    {
      x += (frame.w - tw);
    }
    canvas->drawTextBlob(tb, x, y, pen);

    if (textStyle.isStrikeThrough)
    {
      SkPaint p = pen;
      p.setStrokeWidth(metrics.fStrikeoutThickness);
      canvas->drawLine(x,
                       y + metrics.fStrikeoutPosition,
                       x + tw,
                       y + metrics.fStrikeoutPosition,
                       p);
    }
    if (textStyle.isUnderline)
    {
      SkPaint p = pen;
      p.setStrokeWidth(metrics.fUnderlineThickness);
      canvas->drawLine(x,
                       y + metrics.fUnderlinePosition,
                       x + tw,
                       y + metrics.fUnderlinePosition,
                       p);
    }

    y += (dh + dl);
  }

  canvas->restore();
}

void drawFramedRelation(SkCanvas* canvas, FramedRelation& fr)
{
  ASSERT(canvas);
  auto& frame = fr.frame;
  ASSERT(frame.w > 0);
  ASSERT(frame.h > 0);

  canvas->save();
  canvas->translate(frame.x, frame.y);
  canvas->translate(frame.w / 2, frame.h / 2);
  canvas->scale(frame.flipX ? -1 : 1, frame.flipY ? -1 : 1);
  canvas->rotate(frame.rotation);
  canvas->translate(-frame.w / 2, -frame.h / 2);

  fr.map([&](Entity& ent) { RenderSystem::drawEntity(canvas, ent); });
  canvas->restore();
}

void drawEntity(SkCanvas* canvas, Entity& entity)
{
  if (!entity.visible)
  {
    return;
  }
  if (entity.components.hasRenderable<FramedPath>())
  {
    auto* fp = entity.components.getRenderable<FramedPath>();
    ASSERT(fp);
    RenderSystem::drawFramedPath(canvas, *fp);
  }
  else if (entity.components.hasRenderable<FramedText>())
  {
    auto* ft = entity.components.getRenderable<FramedText>();
    ASSERT(ft);
    RenderSystem::drawFramedText(canvas, *ft);
  }
  else if (entity.components.hasRenderable<FramedRelation>())
  {
    auto* fr = entity.components.getRenderable<FramedRelation>();
    ASSERT(fr);
    RenderSystem::drawFramedRelation(canvas, *fr);
  }
  else
  {
    WARN("No supported renderable found for entity drawing.");
  }
}

void drawEntity(SkCanvas* canvas, MouseEntity& me)
{
  ASSERT(canvas);

  if (me.selectionPoint.has_value())
  {
    SkPaint fillPen;
    fillPen.setStyle(SkPaint::kFill_Style);
    fillPen.setColor(SkColorSetARGB(0x3f, 0xbf, 0xbf, 0xbf));

    SkPaint strokePen;
    strokePen.setStyle(SkPaint::kStroke_Style);
    strokePen.setColor(SK_ColorGRAY);

    if (me.previewEntity.has_value())
    {
      auto& ent = me.previewEntity.value();
      drawEntity(canvas, ent);
    }
    else
    {
      const SkRect& sel = me.getSelectionFrame().toSkRect();
      canvas->drawRect(sel, fillPen);
      canvas->drawRect(sel, strokePen);
    }
  }

  if (me.trajectory.size() > 1)
  {
    SkPaint strokePen;
    strokePen.setStyle(SkPaint::kStroke_Style);
    strokePen.setColor(SK_ColorBLACK);

    SkPath path;

    auto sp = me.trajectory[0];
    path.moveTo(sp.x, sp.y);
    for (size_t i = 1; i < me.trajectory.size(); i++)
    {
      auto& p = me.trajectory[i];
      path.lineTo(p.x, p.y);
    }
    canvas->drawPath(path, strokePen);
  }

  if (me.components.hasRenderable<FramedPath>())
  {
    auto* fp = me.components.getRenderable<FramedPath>();
    ASSERT(fp);
    fp->frame.x = me.x;
    fp->frame.y = me.y;
    RenderSystem::drawFramedPath(canvas, *fp);
  }
  else if (me.components.hasRenderable<FramedRelation>())
  {
    auto* fr = me.components.getRenderable<FramedRelation>();
    ASSERT(fr);
    fr->frame.x = me.x;
    fr->frame.y = me.y;
    RenderSystem::drawFramedRelation(canvas, *fr);
  }
}

}; // namespace RenderSystem

}; // namespace VGG
