#include "Basic/PaintNode.h"
#include "Basic/PathNode.h"
#include "Basic/VGGType.h"
#include "Utils/Utils.hpp"
#include "include/core/SkBlendMode.h"
#include "include/core/SkPath.h"
#include <limits>

namespace VGG
{
constexpr float EPS = std::numeric_limits<float>::epsilon();
double calcRadius(double r0,
                  const glm::vec2& p0,
                  const glm::vec2& p1,
                  const glm::vec2& p2,
                  glm::vec2* left,
                  glm::vec2* right)
{
  glm::vec2 a = p0 - p1;
  glm::vec2 b = p2 - p1;
  double alen = a.length();
  double blen = b.length();
  if (std::fabs(alen) < EPS || std::fabs(blen) < EPS)
  {
    return 0.;
  }
  ASSERT(alen > 0 && blen > 0);
  double cosTheta = glm::dot(a, b) / alen / blen;
  if (cosTheta + 1 < EPS) // cosTheta == -1
  {
    if (left)
    {
      left->x = p1.x;
      left->y = p1.y;
    }
    if (right)
    {
      right->x = p1.x;
      right->y = p1.y;
    }
    return r0;
  }
  else if (1 - cosTheta < EPS) // cosTheta == 1
  {
    return 0.;
  }
  double tanHalfTheta = std::sqrt((1 - cosTheta) / (1 + cosTheta));
  double radius = r0;
  radius = std::min(radius, 0.5 * alen * tanHalfTheta);
  radius = std::min(radius, 0.5 * blen * tanHalfTheta);
  if (left)
  {
    ASSERT(tanHalfTheta > 0);
    float len = radius / tanHalfTheta;
    *left = p1 + float(len / alen) * a;
  }
  if (right)
  {
    ASSERT(tanHalfTheta > 0);
    double len = radius / tanHalfTheta;
    *right = p1 + (float(len / blen) * b);
  }
  return radius;
}

SkPath getSkiaPath(const std::vector<PointAttr>& points, bool isClosed)
{
  constexpr float w = 1.0;
  constexpr float h = 1.0;
  auto& pts = points;

  ASSERT(w > 0);
  ASSERT(h > 0);

  SkPath skPath;

  if (pts.size() < 2)
  {
    // WARN("Too few path points.");
    return skPath;
  }

  using PM = PointMode;
  auto* startP = &pts[0];
  auto* endP = &pts[pts.size() - 1];
  auto* prevP = endP;
  auto* currP = startP;
  auto* nextP = currP + 1;

  const glm::vec2 s = { w, h };

  if (currP->radius > 0 && currP->mode() == PM::STRAIGHT)
  {
    glm::vec2 start = currP->point * s;
    calcRadius(currP->radius,
               prevP->point * s,
               currP->point * s,
               nextP->point * s,
               nullptr,
               &start);
    skPath.moveTo(start.x, start.y);
  }
  else
  {
    skPath.moveTo(w * currP->point.x, h * currP->point.y);
  }

  while (true)
  {
    if (currP->mode() == PM::STRAIGHT && nextP->mode() == PM::STRAIGHT)
    {
      if (nextP->radius > 0 && nextP->mode() == PM::STRAIGHT)
      {
        auto* next2P = (nextP == endP) ? startP : (nextP + 1);
        auto next2Pp = next2P->to.has_value() ? next2P->to.value() : next2P->point;
        double r = calcRadius(nextP->radius, currP->point * s, nextP->point * s, next2Pp * s, 0, 0);
        skPath.arcTo(w * nextP->point.x, h * nextP->point.y, w * next2Pp.x, h * next2Pp.y, r);
      }
      else
      {
        skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
      }
    }
    else if (currP->mode() == PM::DISCONNECTED && nextP->mode() == PM::DISCONNECTED)
    {
      bool hasFrom = currP->from.has_value();
      bool hasTo = nextP->to.has_value();
      if (!hasFrom && !hasTo)
      {
        skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
      }
      else if (hasFrom && !hasTo)
      {
        auto& from = currP->from.value();
        skPath.quadTo(w * from.x, h * from.y, w * nextP->point.x, h * nextP->point.y);
      }
      else if (!hasFrom && hasTo)
      {
        auto& to = nextP->to.value();
        skPath.quadTo(w * to.x, h * to.y, w * nextP->point.x, h * nextP->point.y);
      }
      else
      {
        auto& from = currP->from.value();
        auto& to = nextP->to.value();
        skPath.cubicTo(w * from.x,
                       h * from.y,
                       w * to.x,
                       h * to.y,
                       w * nextP->point.x,
                       h * nextP->point.y);
      }
    }
    else if (currP->mode() != PM::STRAIGHT && nextP->mode() != PM::STRAIGHT)
    {
      if ((currP->mode() == PM::DISCONNECTED && !currP->from.has_value()) ||
          (nextP->mode() == PM::DISCONNECTED && !nextP->to.has_value()) ||
          (currP->mode() != PM::DISCONNECTED &&
           !(currP->from.has_value() && currP->to.has_value())) ||
          (nextP->mode() != PM::DISCONNECTED &&
           !(nextP->from.has_value() && nextP->to.has_value())))
      {
        WARN("Missing control points.");
        return skPath;
      }
      auto& from = currP->from.value();
      auto& to = nextP->to.value();
      skPath.cubicTo(w * from.x,
                     h * from.y,
                     w * to.x,
                     h * to.y,
                     w * nextP->point.x,
                     h * nextP->point.y);
    }
    else if (currP->mode() == PM::STRAIGHT && nextP->mode() != PM::STRAIGHT)
    {
      if (!nextP->to.has_value())
      {
        skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
      }
      else
      {
        auto& to = nextP->to.value();
        skPath.quadTo(w * to.x, h * to.y, w * nextP->point.x, h * nextP->point.y);
      }
    }
    else if (currP->mode() != PM::STRAIGHT && nextP->mode() == PM::STRAIGHT)
    {
      if (nextP->radius > 0 && nextP->mode() == PM::STRAIGHT)
      {
        auto* next2P = (nextP == endP) ? startP : (nextP + 1);
        if (!currP->from.has_value())
        {
          glm::vec2 start;
          double r = calcRadius(nextP->radius,
                                currP->point * s,
                                nextP->point * s,
                                next2P->point * s,
                                &start,
                                nullptr);
          skPath.lineTo(start.x, start.y);
          skPath.arcTo(w * nextP->point.x,
                       h * nextP->point.y,
                       w * next2P->point.x,
                       h * next2P->point.y,
                       r);
        }
        else
        {
          auto currPfrom = currP->from.value();
          constexpr float radius_coeff = 1.0;
          // glm::vec2 p =
          // currP->point.add(currPfrom.sub(currP->point).scale(radius_coeff)).scale(w, h);
          glm::vec2 p = (currP->point + radius_coeff * (currPfrom - currP->point)) * s;
          glm::vec2 start;
          double r =
            calcRadius(nextP->radius, p, nextP->point * s, next2P->point * s, &start, nullptr);
          skPath.quadTo(p.x, p.y, start.x, start.y);
          skPath.arcTo(w * nextP->point.x,
                       h * nextP->point.y,
                       w * next2P->point.x,
                       h * next2P->point.y,
                       r);
        }
      }
      else
      {
        if (!currP->from.has_value())
        {
          skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
        }
        else
        {
          auto& from = currP->from.value();
          skPath.quadTo(w * from.x, h * from.y, w * nextP->point.x, h * nextP->point.y);
        }
      }
    }
    else
    {
      WARN("Invalid point mode combination: %d %d", (int)currP->mode(), (int)nextP->mode());
    }
    currP = nextP;
    nextP = (nextP == endP) ? startP : (nextP + 1);

    if (isClosed)
    {
      if (currP == startP)
      {
        break;
      }
    }
    else
    {
      if (nextP == startP)
      {
        break;
      }
    }
  }

  if (isClosed)
  {
    skPath.close();
  }

  return skPath;
}

// void drawPathFill(SkCanvas* canvas,
//                   const Frame& frame,
//                   const SkPath& skPath,
//                   const FillStyle* style,
//                   double globalAlpha)
// {
//   ASSERT(canvas);
//   SkPaint fillPen;
//   fillPen.setStyle(SkPaint::kFill_Style);
//
//   if (!style)
//   {
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isFlat(*style))
//   {
//     fillPen.setColor(style->color);
//     fillPen.setAlphaf(fillPen.getAlphaf() * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isLinearGradient(*style))
//   {
//     fillPen.setShader(style->gradient.getLinearShader(frame));
//     fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isRadialGradient(*style))
//   {
//     fillPen.setShader(style->gradient.getRadialShader(frame));
//     fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isAngularGradient(*style))
//   {
//     fillPen.setShader(style->gradient.getAngularShader(frame));
//     fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (style->fillType == FillStyle::FillType::IMAGE)
//   {
//     if (auto& name = style->imageName)
//     {
//       fillPen.setShader(style->getImageShader(frame));
//       fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//       canvas->drawPath(skPath, fillPen);
//     }
//   }
// }

PathNode::PathNode(const std::string& name)
  : PaintNode(name, ObjectType::VGG_PATH)
{
}

void PathNode::Paint(SkCanvas* canvas)
{
  if (shape.subshape.contour.has_value())
  {
    drawContour(canvas);
    // auto skpath = getSkiaPath(shape.subshape.contour.value(), shape.subshape.contour->closed);
    // SkPaint p;
    // p.setAntiAlias(true);
    // p.setStyle(SkPaint::kStroke_Style);
    // p.setColor(SK_ColorBLUE);
    // p.setStrokeWidth(2);
    //
    // canvas->save();
    // canvas->scale(1, -1);
    // canvas->drawPath(skpath, p);
    // canvas->restore();
  }
}

void PathNode::drawContour(SkCanvas* canvas)
{
  ASSERT(shape.subshape.contour.has_value());
  auto skPath = getSkiaPath(shape.subshape.contour.value(), shape.subshape.contour->closed);
  SkPaint p;
  p.setAntiAlias(true);
  p.setStyle(SkPaint::kStroke_Style);
  p.setColor(SK_ColorBLUE);
  p.setStrokeWidth(1);

  canvas->save();
  canvas->scale(1, -1);
  // canvas->drawPath(skPath, p);

  // winding rule
  if (shape.windingRule == WindingType::WR_EVENODD)
  {
    skPath.setFillType(SkPathFillType::kEvenOdd);
  }
  else
  {
    skPath.setFillType(SkPathFillType::kWinding);
  }

  const auto globalAlpha = contextSetting.Opacity;

  for (const auto& f : style.fills)
  {
    if (!f.isEnabled)
      continue;
    SkPaint fillPen;
    fillPen.setColor(f.color);
    fillPen.setStyle(SkPaint::kFill_Style);
    fillPen.setAlphaf(fillPen.getAlphaf() * globalAlpha);
    canvas->drawPath(skPath, fillPen);
  }
  canvas->restore();
}

} // namespace VGG
