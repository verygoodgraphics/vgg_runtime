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
#ifndef __CAPPING_PROFILER_HPP__
#define __CAPPING_PROFILER_HPP__

#include <array>
#include <chrono>
#include <stdio.h>

namespace VGG
{

class CappingProfiler
{
public:
  using clock = std::chrono::steady_clock;
  using dms = std::chrono::duration<double, std::milli>;
  using tps = std::chrono::time_point<clock, dms>;

  template<typename T, int N>
  class RingBuffer
  {
    std::array<T, N> data;
    size_t idx{ 0 };
    int num{ 0 };

  public:
    RingBuffer& push_back(const T& v)
    {
      data[idx] = v;
      idx = (idx + 1) % N;
      num = std::min(N, num + 1);
      return *this;
    }

    T average() const
    {
      T sum{ 0 };
      for (int n = num; n > 0; n--)
      {
        sum += data[(idx - n) % N];
      }
      return num == 0 ? 0 : sum / num;
    }
  };

  static CappingProfiler* getInstance()
  {
    static CappingProfiler profiler;
    return &profiler;
  }

  inline bool enoughFrameDuration(double fps = 60.)
  {
    tps now = clock::now();
    dms d = now - m_timestamp;
    return (d.count() > 1000. / fps);
  }

  inline void markFrame()
  {
    tps now = clock::now();
    dms d = now - m_timestamp;
    m_durations.push_back(d.count());
    m_timestamp = now;
  }

  inline const char* fpsStr()
  {
    static char buf[20];
    snprintf(buf, 20, "%.2lf fps", 1000. / m_durations.average());
    return buf;
  }

private:
  RingBuffer<double, 20> m_durations;
  tps m_timestamp;

  CappingProfiler()
    : m_timestamp(clock::now())
  {
  }
};

}; // namespace VGG

#endif // __CAPPING_PROFILER_HPP__
