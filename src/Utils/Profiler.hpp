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
#ifndef __PROFILER_HPP__
#define __PROFILER_HPP__

#include <skia/include/core/SkCanvas.h>
#include <skia/include/core/SkColor.h>
#include <skia/include/core/SkFont.h>
#include <skia/include/core/SkTime.h>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Utils/Utils.hpp"

namespace VGG
{

class Profiler
{
public:
  static const int N_MEASURES = 1 << 6;
  static const int WIDTH = 200;
  static const int HEIGHT = 100;
  static const int TEXT_HEIGHT = 18;

private:
  struct TimeSegment
  {
    double currTimeStamp;
    double accumulatedTime;
  };
  typedef std::unordered_map<std::string, TimeSegment> TimeMeasure;

  bool m_inited;
  bool m_full;
  size_t m_currMeasure;
  std::vector<TimeMeasure> m_measures;
  std::vector<std::pair<std::string, SkColor>> m_segColors;

  Profiler()
    : m_inited(false)
    , m_full(false)
    , m_currMeasure(0)
  {
  }

  static bool init(Profiler* profiler)
  {
    ASSERT(profiler);
    if (profiler->m_inited)
    {
      return true;
    }
    profiler->reset();
    profiler->m_inited = true;
    return true;
  }

  void buildColorMapping(const std::string& name)
  {
    static const int NCOLORS = 8;
    static const SkColor colors[NCOLORS] = {
      SK_ColorMAGENTA, SK_ColorCYAN, SK_ColorYELLOW, SK_ColorBLUE,
      SK_ColorGREEN,   SK_ColorRED,  SK_ColorBLACK,  SK_ColorWHITE,
    };
    size_t n = m_segColors.size();
    for (size_t i = 0; i < n; i++)
    {
      if (m_segColors[i].first == name)
      {
        return;
      }
    }
    if (n < NCOLORS)
    {
      m_segColors.push_back(std::pair<std::string, SkColor>(name, colors[n]));
    }
  }

public:
  std::string getAverageTimeString(const std::string& label = "")
  {
    double totalTime = 0.0;
    int count = 0;
    for (size_t i = 0; i < N_MEASURES; i++)
    {
      auto& tm = m_measures[i];
      size_t nSegs = tm.size();
      if (nSegs > 0)
      {
        count += 1;
        for (auto j = tm.begin(); j != tm.end(); j++)
        {
          totalTime += j->second.accumulatedTime;
        }
      }
    }
    if (count == 0)
    {
      return "Average: NA";
    }
    std::ostringstream ss;
    if (label.empty())
    {
      ss << "Average: ";
    }
    else
    {
      ss << label << ": ";
    }
    ss << std::setiosflags(std::ios::fixed | std::ios::right) << std::setprecision(2)
       << std::setw(5);
    ss << totalTime / count << "ms";
    double fps = 1000.0 / totalTime * count;
    ss << std::setprecision(0) << " (" << fps << "fps)";
    return ss.str();
  }

  std::string getAverageTimeStringOf(const std::string& name)
  {
    double totalTime = 0.0;
    int count = 0;
    for (size_t i = 0; i < N_MEASURES; i++)
    {
      auto& tm = m_measures[i];
      size_t nSegs = tm.size();
      if (nSegs > 0 && tm.find(name) != tm.end())
      {
        count += 1;
        totalTime += tm[name].accumulatedTime;
      }
    }
    std::ostringstream ss;
    ss << "Average " << name << ": ";
    if (count == 0)
    {
      ss << "NA";
      return ss.str();
    }
    ss << std::setiosflags(std::ios::fixed | std::ios::right) << std::setprecision(2)
       << std::setw(5);
    ss << totalTime / count << "ms";
    return ss.str();
  }

public:
  void reset()
  {
    m_full = false;
    m_currMeasure = 0;
    m_measures.clear();
    m_measures.resize(N_MEASURES);
    m_segColors.clear();
  }

  void startTiming(const std::string& name)
  {
    m_measures[m_currMeasure][name] = TimeSegment{
      .currTimeStamp = SkTime::GetMSecs(),
      .accumulatedTime = 0.0,
    };
    buildColorMapping(name);
  }

  void stopTiming(const std::string& name)
  {
    auto& tm = m_measures[m_currMeasure];
    if (tm.find(name) == tm.end())
    {
      WARN("No previous timing found: %s", name.c_str());
      return;
    }
    double ct = SkTime::GetMSecs();
    double dt = ct - tm[name].currTimeStamp;
    tm[name].currTimeStamp = ct;
    tm[name].accumulatedTime += dt;
  }

  void nextMeasure()
  {
    m_currMeasure = (m_currMeasure + 1) % N_MEASURES;
    m_measures[m_currMeasure].clear();
    m_full = m_full || (m_currMeasure == 0);
  }

  void draw(SkCanvas* canvas)
  {
    ASSERT(canvas);
    SkPaint paint;
    paint.setAntiAlias(true);

    // draw box
    paint.setColor(SK_ColorDKGRAY);
    canvas->drawRect({ 0, 0, WIDTH, HEIGHT }, paint);

    // draw text
    SkFont font(nullptr, 13);
    double textHeight = TEXT_HEIGHT;
    paint.setColor(SK_ColorWHITE);
    canvas->drawString(getAverageTimeString().c_str(), 5, textHeight, font, paint);
    for (auto it = m_segColors.begin(); it != m_segColors.end(); it++)
    {
      paint.setColor(it->second);
      textHeight += TEXT_HEIGHT;
      canvas->drawString(getAverageTimeStringOf(it->first).c_str(), 5, textHeight, font, paint);
    }

    // draw (stacked) bars
    double dw = (double)WIDTH / N_MEASURES;
    double padding = dw * 0.2;
    double chartHeight = HEIGHT - textHeight;
    double dh = (chartHeight - 2.0 * padding) / (1000.0 / 30.0);
    size_t offset = m_full ? 0 : (N_MEASURES - m_currMeasure);
    for (size_t i = offset; i < N_MEASURES; i++)
    {
      size_t idx = m_full ? ((i + m_currMeasure) % N_MEASURES) : (i - offset);
      double h = 0.0;
      for (auto it = m_segColors.begin(); it != m_segColors.end(); it++)
      {
        paint.setColor(it->second);
        double th = dh * m_measures[idx][it->first].accumulatedTime;
        canvas->drawRect(
          {
            (SkScalar)(dw * i + padding),
            (SkScalar)(HEIGHT - padding - h),
            (SkScalar)(dw * (i + 1) - padding),
            (SkScalar)(HEIGHT - padding - h - th),
          },
          paint);
        h += th;
      }
    }

    // draw 60fps reference line
    paint.setColor(SK_ColorGRAY);
    double rh = HEIGHT - padding - 1000.0 / 60.0 * dh;
    canvas->drawLine(0, rh, WIDTH, rh, paint);
  }

  static Profiler* getInstance()
  {
    static Profiler profiler;
    if (!init(&profiler))
    {
      return nullptr;
    }
    return &profiler;
  }
}; // class Profiler

}; // namespace VGG

#endif // __PROFILER_HPP__
