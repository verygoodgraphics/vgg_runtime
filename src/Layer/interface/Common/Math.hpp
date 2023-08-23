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
#ifndef __MATH_HPP__
#define __MATH_HPP__

#include <cmath>
#include <vector>

#include <Utility/interface/Log.h>

#define FZERO 1e-10
#define FTOLER 1e-2

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

namespace VGG
{

struct Vec2
{
  double x{ 0. };
  double y{ 0. };

  inline double dot(const Vec2& ov) const
  {
    return x * ov.x + y * ov.y;
  }

  inline double len() const
  {
    return std::sqrt(x * x + y * y);
  }

  inline Vec2 add(const Vec2& ov) const
  {
    return Vec2{ x + ov.x, y + ov.y };
  }

  inline Vec2 sub(const Vec2& ov) const
  {
    return Vec2{ x - ov.x, y - ov.y };
  }

  inline Vec2 scale(double s) const
  {
    return Vec2{ s * x, s * y };
  }

  inline Vec2 scale(double s, double t) const
  {
    return Vec2{ s * x, t * y };
  }
};

inline Vec2 operator+(const Vec2& a, const Vec2& b)
{
  return a.add(b);
}

inline Vec2 operator-(const Vec2& a, const Vec2& b)
{
  return a.sub(b);
}

inline Vec2 operator*(const Vec2& a, double b)
{
  return a.scale(b);
}

inline Vec2 operator/(const Vec2& a, double b)
{
  ASSERT(std::fabs(b) > FZERO);
  return a.scale(1. / b);
}

template<typename T>
inline T lerp(const T& a, const T& b, double t)
{
  return a + (b - a) * t;
}

inline std::vector<double> getBairstowPolyRoots(const std::vector<double>& coeffs)
{
  std::vector<double> roots;
  std::vector<double> a{ coeffs };

  while (a.size() > 0 && std::fabs(a[a.size() - 1]) < FZERO)
  {
    a.pop_back();
  }
  if (a.size() == 0)
  {
    return roots;
  }
  size_t deg = a.size() - 1;
#define DEG1_CASE                                                                                  \
  if (deg == 1)                                                                                    \
  {                                                                                                \
    ASSERT(std::fabs(a[1]) > FZERO);                                                               \
    roots.push_back(-a[0] / a[1]);                                                                 \
    return roots;                                                                                  \
  }
#define DEG2_CASE                                                                                  \
  if (deg == 2)                                                                                    \
  {                                                                                                \
    ASSERT(std::fabs(a[2]) > FZERO);                                                               \
    double det = a[1] * a[1] - 4 * a[0] * a[2];                                                    \
    if (std::fabs(det) < FZERO)                                                                    \
    {                                                                                              \
      roots.push_back(-a[1] / (2 * a[2]));                                                         \
    }                                                                                              \
    else if (det > FZERO)                                                                          \
    {                                                                                              \
      double sd = std::sqrt(det);                                                                  \
      roots.push_back((-a[1] - sd) / (2 * a[2]));                                                  \
      roots.push_back((-a[1] + sd) / (2 * a[2]));                                                  \
    }                                                                                              \
    return roots;                                                                                  \
  }
  DEG1_CASE
  DEG2_CASE
  while (deg >= 3)
  {
    double r = 0.1;
    double s = 0.1;
    std::vector<double> b(deg + 1);

    for (int iter = 0; iter < 300; iter++)
    {
      {
        b[deg] = a[deg];
        b[deg - 1] = a[deg - 1] - r * b[deg];
        size_t i = deg - 2;
        while (i > 0)
        {
          b[i] = a[i] - r * b[i + 1] - s * b[i + 2];
          i--;
        }
        b[0] = a[0] - s * b[2];
      }

      std::vector<double> c(deg + 1);
      {
        c[deg] = b[deg];
        c[deg - 1] = b[deg - 1] - r * c[deg];
        size_t i = deg - 2;
        while (i > 1)
        {
          c[i] = b[i] - r * c[i + 1] - s * c[i + 2];
          i--;
        }
        c[1] = -s * c[3];
      }

      std::vector<double> d(deg + 1);
      {
        d[deg] = b[deg];
        d[deg - 1] = b[deg - 1] - r * d[deg];
        size_t i = deg - 2;
        while (i > 2)
        {
          d[i] = b[i] - r * d[i + 1] - s * d[i + 2];
          i--;
        }
        d[2] = b[2] - s * d[4];
      }

      double dr = (b[0] * d[3] - b[1] * d[2]) / ((d[2] - r * d[3]) * d[2] + s * d[3] * d[3]);
      double ds = (-b[1] * s * d[3] - b[0] * (d[2] - r * d[3])) /
                  ((d[2] - r * d[3]) * d[2] + s * d[3] * d[3]);
      if (std::fabs(dr) < FZERO && std::fabs(ds) < FZERO)
      {
        break;
      }
      r = r - dr;
      s = s - ds;
    }

    {
      double det = r * r - 4 * s;
      if (std::fabs(det) < FZERO)
      {
        roots.push_back(-r / 2);
      }
      else if (det > FZERO)
      {
        double sd = std::sqrt(det);
        roots.push_back((-r - sd) / 2);
        roots.push_back((-r + sd) / 2);
      }
    }

    a.clear();
    a.assign(b.begin() + 2, b.end());
    deg = deg - 2;
  }
  DEG2_CASE
  DEG1_CASE
#undef DEG1_CASE
#undef DEG2_CASE
  return roots;
}

inline double deg2rad(double deg)
{
  return deg / 180. * M_PI;
}

inline double rad2deg(double rad)
{
  return rad / M_PI * 180.;
}

template<typename T>
inline void clampPairByLimits(T& low,
                              T& high,
                              const T& lowLimit,
                              const T& highLimit,
                              const T& precision)
{
  if (low >= highLimit)
  {
    low -= precision;
  }
  else if (high < lowLimit)
  {
    high += precision;
  }
  else if (low >= high)
  {
    low -= precision / 2;
    high += precision / 2;
  }
  ASSERT(low >= lowLimit && high <= highLimit);
  ASSERT(low < high);
}

}; // namespace VGG

#endif // __MATH_HPP__
