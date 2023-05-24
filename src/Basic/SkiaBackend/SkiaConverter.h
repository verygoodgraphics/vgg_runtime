#pragma once
#include "include/core/SkPaint.h"
#include "Basic/VGGType.h"
#include "Basic/Attrs.h"
#include "include/pathops/SkPathOps.h"

#include <vector>

using namespace VGG;

inline SkPaint::Join toSkPaintJoin(VGG::ELineJoin join)
{
  switch (join)
  {
    case VGG::LJ_Miter:
      return SkPaint::kMiter_Join;
    case VGG::LJ_Round:
      return SkPaint::kRound_Join;
    case VGG::LJ_Bevel:
      return SkPaint::kBevel_Join;
  }
  return SkPaint::kMiter_Join;
}

inline SkPaint::Cap toSkPaintCap(VGG::ELineCap cap)
{
  switch (cap)
  {
    case VGG::LC_Butt:
      return SkPaint::kButt_Cap;
    case VGG::LC_Round:
      return SkPaint::kRound_Cap;
    case VGG::LC_Square:
      return SkPaint::kSquare_Cap;
  }
  return SkPaint::kButt_Cap;
}

inline SkPathOp toSkPathOp(VGG::EBoolOp blop)
{
  SkPathOp op;
  switch (blop)
  {
    case VGG::BO_Union:
      op = SkPathOp::kUnion_SkPathOp;
      break;
    case VGG::BO_Substraction:
      op = SkPathOp::kDifference_SkPathOp;
      break;
    case VGG::BO_Intersection:
      op = SkPathOp::kIntersect_SkPathOp;
      break;
    case VGG::BO_Exclusion:
      op = SkPathOp::kXOR_SkPathOp;
      break;
    default:
      return SkPathOp::kUnion_SkPathOp;
  }
  return op;
}

constexpr float EPS = std::numeric_limits<float>::epsilon();

inline double calcRadius(double r0,
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

inline SkPath getSkiaPath(const std::vector<PointAttr>& points, bool isClosed)
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

  using PM = EPointMode;
  auto* startP = &pts[0];
  auto* endP = &pts[pts.size() - 1];
  auto* prevP = endP;
  auto* currP = startP;
  auto* nextP = currP + 1;

  const glm::vec2 s = { w, h };

  if (currP->radius > 0 && currP->mode() == PM::PM_Straight)
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
    if (currP->mode() == PM::PM_Straight && nextP->mode() == PM::PM_Straight)
    {
      if (nextP->radius > 0 && nextP->mode() == PM::PM_Straight)
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
    else if (currP->mode() == PM::PM_Disconnected && nextP->mode() == PM::PM_Disconnected)
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
    else if (currP->mode() != PM::PM_Straight && nextP->mode() != PM::PM_Straight)
    {
      if ((currP->mode() == PM::PM_Disconnected && !currP->from.has_value()) ||
          (nextP->mode() == PM::PM_Disconnected && !nextP->to.has_value()) ||
          (currP->mode() != PM::PM_Disconnected &&
           !(currP->from.has_value() && currP->to.has_value())) ||
          (nextP->mode() != PM::PM_Disconnected &&
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
    else if (currP->mode() == PM::PM_Straight && nextP->mode() != PM::PM_Straight)
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
    else if (currP->mode() != PM::PM_Straight && nextP->mode() == PM::PM_Straight)
    {
      if (nextP->radius > 0 && nextP->mode() == PM::PM_Straight)
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
