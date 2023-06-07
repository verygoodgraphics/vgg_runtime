/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
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
#include "Systems/GeometrySystem.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Math.hpp"
#include "Utils/RadiusCoeff.hpp"

//#define DEBUG2(...) DEBUG(__VA_ARGS__)
#define DEBUG2(...)

namespace VGG
{

namespace GeometrySystem
{

double calcRadius(double r0,
                  const Vec2& p0,
                  const Vec2& p1,
                  const Vec2& p2,
                  Vec2* left,
                  Vec2* right)
{
  Vec2 a = p0.sub(p1);
  Vec2 b = p2.sub(p1);
  double alen = a.len();
  double blen = b.len();
  if (std::fabs(alen) < FZERO || std::fabs(blen) < FZERO)
  {
    return 0.;
  }
  ASSERT(alen > 0 && blen > 0);
  double cosTheta = a.dot(b) / alen / blen;
  if (cosTheta + 1 < FZERO) // cosTheta == -1
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
  else if (1 - cosTheta < FZERO) // cosTheta == 1
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
    double len = radius / tanHalfTheta;
    *left = p1.add(a.scale(len / alen));
  }
  if (right)
  {
    ASSERT(tanHalfTheta > 0);
    double len = radius / tanHalfTheta;
    *right = p1.add(b.scale(len / blen));
  }
  return radius;
}

SkPath getSkiaPath(const FramedPath& fp)
{
  DEBUG2("----------------------------------------------- getSkiaPath");
  auto& frame = fp.frame;
  auto& path = fp.path;
  auto& pts = path.points;
  auto w = frame.w;
  auto h = frame.h;

  ASSERT(frame.w > 0);
  ASSERT(frame.h > 0);

  SkPath skPath;

  if (pts.size() < 2)
  {
    // WARN("Too few path points.");
    return skPath;
  }

  auto* startP = &pts[0];
  auto* endP = &pts[pts.size() - 1];
  auto* prevP = endP;
  auto* currP = startP;
  auto* nextP = currP + 1;

  if (currP->radius > 0 && currP->mode == PM::STRAIGHT)
  {
    Vec2 start = currP->point.scale(w, h);
    calcRadius(currP->radius,
               prevP->point.scale(w, h),
               currP->point.scale(w, h),
               nextP->point.scale(w, h),
               nullptr,
               &start);
    DEBUG2("BEGIN: moveTo(%.2lf, %.2lf)", start.x, start.y);
    skPath.moveTo(start.x, start.y);
  }
  else
  {
    DEBUG2("BEGIN: moveTo(%.2lf, %.2lf)", w * currP->point.x, h * currP->point.y);
    skPath.moveTo(w * currP->point.x, h * currP->point.y);
  }

  while (true)
  {
    if (currP->mode == PM::STRAIGHT && nextP->mode == PM::STRAIGHT)
    {
      if (nextP->radius > 0 && nextP->mode == PM::STRAIGHT)
      {
        auto* next2P = (nextP == endP) ? startP : (nextP + 1);
        auto next2Pp = next2P->to.has_value() ? next2P->to.value() : next2P->point;
        double r = calcRadius(nextP->radius,
                              currP->point.scale(w, h),
                              nextP->point.scale(w, h),
                              next2Pp.scale(w, h));
        DEBUG2("STRAIGHT->STRAIGHT: arcTo(%.2lf, %.2lf, %.2lf, %.2lf, %.2lf)",
               w * nextP->point.x,
               h * nextP->point.y,
               w * next2Pp.x,
               h * next2Pp.y r);
        skPath.arcTo(w * nextP->point.x, h * nextP->point.y, w * next2Pp.x, h * next2Pp.y, r);
      }
      else
      {
        DEBUG2("STRAIGHT->STRAIGHT: lineTo(%.2lf, %.2lf)", w * nextP->point.x, h * nextP->point.y);
        skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
      }
    }
    else if (currP->mode == PM::DISCONNECTED && nextP->mode == PM::DISCONNECTED)
    {
      bool hasFrom = currP->from.has_value();
      bool hasTo = nextP->to.has_value();
      if (!hasFrom && !hasTo)
      {
        DEBUG2("DISCONNECTED->DISCONNECTED: lineTo(%.2lf, %.2lf)",
               w * nextP->point.x,
               h * nextP->point.y);
        skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
      }
      else if (hasFrom && !hasTo)
      {
        auto& from = currP->from.value();
        DEBUG2("DISCONNECTED->DISCONNECTED: quadTo(%.2lf, %.2lf, %.2lf, %.2lf)",
               w * from.x,
               h * from.y,
               w * nextP->point.x,
               h * nextP->point.y);
        skPath.quadTo(w * from.x, h * from.y, w * nextP->point.x, h * nextP->point.y);
      }
      else if (!hasFrom && hasTo)
      {
        auto& to = nextP->to.value();
        DEBUG2("DISCONNECTED->DISCONNECTED: quadTo(%.2lf, %.2lf, %.2lf, %.2lf)",
               w * to.x,
               h * to.y,
               w * nextP->point.x,
               h * nextP->point.y);
        skPath.quadTo(w * to.x, h * to.y, w * nextP->point.x, h * nextP->point.y);
      }
      else
      {
        auto& from = currP->from.value();
        auto& to = nextP->to.value();
        DEBUG2("DISCONNECTED->DISCONNECTED: cubicTo(%.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf)",
               w * from.x,
               h * from.y,
               w * to.x,
               h * to.y,
               w * nextP->point.x,
               h * nextP->point.y);
        skPath.cubicTo(w * from.x,
                       h * from.y,
                       w * to.x,
                       h * to.y,
                       w * nextP->point.x,
                       h * nextP->point.y);
      }
    }
    else if (currP->mode != PM::STRAIGHT && nextP->mode != PM::STRAIGHT)
    {
      if ((currP->mode == PM::DISCONNECTED && !currP->from.has_value()) ||
          (nextP->mode == PM::DISCONNECTED && !nextP->to.has_value()) ||
          (currP->mode != PM::DISCONNECTED &&
           !(currP->from.has_value() && currP->to.has_value())) ||
          (nextP->mode != PM::DISCONNECTED && !(nextP->from.has_value() && nextP->to.has_value())))
      {
        WARN("Missing control points.");
        return skPath;
      }
      auto& from = currP->from.value();
      auto& to = nextP->to.value();
      DEBUG2("!STRAIGHT->!STRAIGHT: cubicTo(%.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf)",
             w * from.x,
             h * from.y,
             w * to.x,
             h * to.y,
             w * nextP->point.x,
             h * nextP->point.y);
      skPath.cubicTo(w * from.x,
                     h * from.y,
                     w * to.x,
                     h * to.y,
                     w * nextP->point.x,
                     h * nextP->point.y);
    }
    else if (currP->mode == PM::STRAIGHT && nextP->mode != PM::STRAIGHT)
    {
      if (!nextP->to.has_value())
      {
        DEBUG2("STRAIGHT->!STRAIGHT: lineTo(%.2lf, %.2lf)", w * nextP->point.x, h * nextP->point.y);
        skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
      }
      else
      {
        auto& to = nextP->to.value();
        DEBUG2("STRAIGHT->!STRAIGHT: quadTo(%.2lf, %.2lf, %.2lf, %.2lf)",
               w * to.x,
               h * to.y,
               w * nextP->point.x,
               h * nextP->point.y);
        skPath.quadTo(w * to.x, h * to.y, w * nextP->point.x, h * nextP->point.y);
      }
    }
    else if (currP->mode != PM::STRAIGHT && nextP->mode == PM::STRAIGHT)
    {
      if (nextP->radius > 0 && nextP->mode == PM::STRAIGHT)
      {
        auto* next2P = (nextP == endP) ? startP : (nextP + 1);
        if (!currP->from.has_value())
        {
          Vec2 start;
          double r = calcRadius(nextP->radius,
                                currP->point.scale(w, h),
                                nextP->point.scale(w, h),
                                next2P->point.scale(w, h),
                                &start);
          DEBUG2("!STRAIGHT->STRAIGHT: lineTo(%.2lf, %.2lf)", start.x, start.y);
          skPath.lineTo(start.x, start.y);
          DEBUG2("!STRAIGHT->STRAIGHT: arcTo(%.2lf, %.2lf, %.2lf, %.2lf, %.2lf)",
                 w * nextP->point.x,
                 h * nextP->point.y,
                 w * next2P->point.x,
                 h * next2P->point.y,
                 r);
          skPath.arcTo(w * nextP->point.x,
                       h * nextP->point.y,
                       w * next2P->point.x,
                       h * next2P->point.y,
                       r);
        }
        else
        {
          auto currPfrom = currP->from.value();
          Vec2 p =
            currP->point.add(currPfrom.sub(currP->point).scale(RadiusCoeff::coeff)).scale(w, h);
          Vec2 start;
          double r = calcRadius(nextP->radius,
                                p,
                                nextP->point.scale(w, h),
                                next2P->point.scale(w, h),
                                &start);
          DEBUG2("!STRAIGHT->STRAIGHT: quadTo(%.2lf, %.2lf, %.2lf, %.2lf)",
                 p.x,
                 p.y,
                 start.x,
                 start.y);
          skPath.quadTo(p.x, p.y, start.x, start.y);
          DEBUG2("                     arcTo(%.2lf, %.2lf, %.2lf, %.2lf, %.2lf)",
                 w * nextP->point.x,
                 h * nextP->point.y,
                 w * next2P->point.x,
                 h * next2P->point.y,
                 r);
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
          DEBUG2("!STRAIGHT->STRAIGHT: lineTo(%.2lf, %.2lf)",
                 w * nextP->point.x,
                 h * nextP->point.y);
          skPath.lineTo(w * nextP->point.x, h * nextP->point.y);
        }
        else
        {
          auto& from = currP->from.value();
          DEBUG2("!STRAIGHT->STRAIGHT: quadTo(%.2lf, %.2lf, %.2lf, %.2lf)",
                 w * from.x,
                 h * from.y,
                 w * nextP->point.x,
                 h * nextP->point.y);
          skPath.quadTo(w * from.x, h * from.y, w * nextP->point.x, h * nextP->point.y);
        }
      }
    }
    else
    {
      WARN("Invalid point mode combination: %d %d", (int)currP->mode, (int)nextP->mode);
    }
    currP = nextP;
    nextP = (nextP == endP) ? startP : (nextP + 1);

    if (path.isClosed)
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

  if (path.isClosed)
  {
    skPath.close();
  }

  return skPath;
}

SkPath getSkiaStrokePath(const SkPath& sp)
{
  SkPath strokePath;
  SkPaint pen;
  pen.setStyle(SkPaint::kStroke_Style);
  pen.setStrokeWidth(2 * CurvePoint::SIZE);
  pen.getFillPath(sp, &strokePath);
  return strokePath;
}

std::optional<Vec2> findClosePointToLine(const Vec2& m, const Vec2& p0, const Vec2& p1)
{
  double dx = p1.x - p0.x;
  double dy = p1.y - p0.y;
  double dxm = m.x - p0.x;
  double dym = m.y - p0.y;
  double x = dx * dx * m.x + dy * dy * p0.x + dx * dy * dym;
  double y = dy * dy * m.y + dx * dx * p0.y + dx * dy * dxm;
  double d = dx * dx + dy * dy;
  ASSERT(d > 0);
  return Vec2{ x / d, y / d };
}

std::optional<Vec2> findClosePointToQuad(const Vec2& m,
                                         const Vec2& p0,
                                         const Vec2& p1,
                                         const Vec2& p2)
{
  double Ax = p0.x - 2 * p1.x + p2.x;
  double Ay = p0.y - 2 * p1.y + p2.y;
  double Bx = -2 * p0.x + 2 * p1.x;
  double By = -2 * p0.y + 2 * p1.y;
  double Cx = p0.x;
  double Cy = p0.y;
  double t3 = -2 * Ax * Ax - 2 * Ay * Ay;
  double t2 = -3 * Ax * Bx - 3 * Ay * By;
  double t1 = -Bx * Bx + 2 * Ax * (m.x - Cx) - By * By + 2 * Ay * (m.y - Cy);
  double t0 = (m.x - Cx) * Bx + (m.y - Cy) * By;
  auto roots = getBairstowPolyRoots({ t0, t1, t2, t3 });
  if (roots.size() > 0)
  {
    std::vector<double> validRoots;
    for (size_t i = 0; i < roots.size(); i++)
    {
      if (roots[i] > 0 && roots[i] < 1)
      {
        validRoots.push_back(roots[i]);
      }
    }
    if (validRoots.size() > 0)
    {
      size_t i = 0;
      double t = validRoots[i];
      Vec2 p{ Ax * t * t + Bx * t + Cx, Ay * t * t + By * t + Cy };
      double minDist = p.sub(m).len();
      while (++i < validRoots.size())
      {
        double t = validRoots[i];
        Vec2 tp{ Ax * t * t + Bx * t + Cx, Ay * t * t + By * t + Cy };
        double dist = tp.sub(m).len();
        if (dist < minDist)
        {
          minDist = dist;
          p = tp;
        }
      }
      return p;
    }
  }
  return std::nullopt;
}

std::optional<Vec2> findClosePointToCube(const Vec2& m,
                                         const Vec2& p0,
                                         const Vec2& p1,
                                         const Vec2& p2,
                                         const Vec2& p3)
{
  double Ax = -p0.x + 3 * p1.x - 3 * p2.x + p3.x;
  double Ay = -p0.y + 3 * p1.y - 3 * p2.y + p3.y;
  double Bx = 3 * p0.x - 6 * p1.x + 3 * p2.x;
  double By = 3 * p0.y - 6 * p1.y + 3 * p2.y;
  double Cx = -3 * p0.x + 3 * p1.x;
  double Cy = -3 * p0.y + 3 * p1.y;
  double Dx = p0.x;
  double Dy = p0.y;
  double t5 = -3 * Ax * Ax - 3 * Ay * Ay;
  double t4 = -5 * Ax * Bx - 5 * Ay * By;
  double t3 = -4 * Ax * Cx - 2 * Bx * Bx - 4 * Ay * Cy - 2 * By * By;
  double t2 = -3 * Bx * Cx + 3 * Ax * (m.x - Dx) - 3 * By * Cy + 3 * Ay * (m.y - Dy);
  double t1 = -Cx * Cx + 2 * Bx * (m.x - Dx) - Cy * Cy + 2 * By * (m.y - Dy);
  double t0 = (m.x - Dx) * Cx + (m.y - Dy) * Cy;
  auto roots = getBairstowPolyRoots({ t0, t1, t2, t3, t4, t5 });
  if (roots.size() > 0)
  {
    std::vector<double> validRoots;
    for (size_t i = 0; i < roots.size(); i++)
    {
      if (roots[i] > 0 && roots[i] < 1)
      {
        validRoots.push_back(roots[i]);
      }
    }
    if (validRoots.size() > 0)
    {
      size_t i = 0;
      double t = validRoots[i];
      Vec2 p{ Ax * t * t * t + Bx * t * t + Cx * t + Dx,
              Ay * t * t * t + By * t * t + Cy * t + Dy };
      double minDist = p.sub(m).len();
      while (++i < validRoots.size())
      {
        double t = validRoots[i];
        Vec2 tp{ Ax * t * t * t + Bx * t * t + Cx * t + Dx,
                 Ay * t * t * t + By * t * t + Cy * t + Dy };
        double dist = tp.sub(m).len();
        if (dist < minDist)
        {
          minDist = dist;
          p = tp;
        }
      }
      return p;
    }
  }
  return std::nullopt;
}

double perpendicularDistance(const Vec2& p, const Vec2& a, const Vec2& b)
{
  Vec2 pa = p - a;
  Vec2 pb = p - b;
  double area = std::fabs(pa.x * pb.y - pa.y * pb.x);
  double ab = (a - b).len();
  return area / ab;
}

std::vector<Vec2> simplifyByDouglasPeucker(const std::vector<Vec2>& pts,
                                           size_t i,
                                           size_t j,
                                           double epsilon)
{
  ASSERT(i <= j);
  if (i == j)
  {
    std::vector<Vec2> result;
    result.push_back(pts[i]);
    return result;
  }

  double maxD = 0.0;
  size_t idx = i + 1;
  auto& pi = pts[i];
  auto& pj = pts[j];
  for (size_t k = idx; k < j; k++)
  {
    auto d = perpendicularDistance(pts[k], pi, pj);
    if (d > maxD)
    {
      maxD = d;
      idx = k;
    }
  }

  if (maxD > epsilon)
  {
    auto resa = simplifyByDouglasPeucker(pts, i, idx - 1, epsilon);
    auto resb = simplifyByDouglasPeucker(pts, idx, j, epsilon);
    resa.insert(resa.end(), resb.begin(), resb.end());
    return resa;
  }
  else
  {
    std::vector<Vec2> result;
    result.push_back(pts[i]);
    result.push_back(pts[j]);
    return result;
  }
}

}; // namespace GeometrySystem

}; // namespace VGG
