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

#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <functional>
namespace VGG::layer
{
namespace time_unit_trait
{
struct S
{
  using type = std::ratio<1>;          // NOLINT
  static constexpr const char* c_str() // NOLINT
  {
    return "s";
  }
};
struct Ms
{
  using type = std::milli;             // NOLINT
  static constexpr const char* c_str() // NOLINT
  {
    return "ms";
  }
};
struct Us
{
  using type = std::micro;             // NOLINT
  static constexpr const char* c_str() // NOLINT
  {
    return "us";
  }
};
struct Ns
{
  using type = std::nano;              // NOLINT
  static constexpr const char* c_str() // NOLINT
  {
    return "ns";
  }
};

} // namespace time_unit_trait
template<typename Unit>
struct UnittedDuration final
{
  auto cnt() const
  {
    return _.count();
  }
  operator double() const
  {
    return cnt();
  }
  std::string fmt() const
  {
    std::ostringstream os;
    os << cnt() << "(" << Unit::c_str() << ")";
    return os.str();
  }

  UnittedDuration(
    std::chrono::duration<double, typename std::chrono::system_clock::duration::period> const& _)
    : _(std::chrono::duration_cast<decltype(this->_)>(_))
  {
  }

public:
  friend std::ostream& operator<<(std::ostream& os, UnittedDuration const& _)
  {
    return os << _.fmt();
  }

private:
  std::chrono::duration<double, typename Unit::type> _; // NOLINT
};

struct Duration final
{
  auto s() const
  {
    return UnittedDuration<time_unit_trait::S>(_);
  }
  auto ms() const
  {
    return UnittedDuration<time_unit_trait::Ms>(_);
  }
  auto us() const
  {
    return UnittedDuration<time_unit_trait::Us>(_);
  }
  auto ns() const
  {
    return UnittedDuration<time_unit_trait::Ns>(_);
  }

  Duration() = default;
  template<typename Rep, typename Period>
  Duration(std::chrono::duration<Rep, Period> const& _)
    : _(_)
  {
  }

public:
  friend std::ostream& operator<<(std::ostream& os, Duration const& _)
  {
    return os << _.ms();
  }

private:
  std::chrono::duration<double, typename std::chrono::system_clock::duration::period> _; // NOLINT
};

struct TimePoint
{
  /*
   * see the std::put_time reference for more detail about the fmt string
   * */
  auto to(const char* fmt) const
  {
    return std::put_time(localtime(&_), fmt);
  }

  auto cnt() const
  {
    return _;
  }
  friend std::ostream& operator<<(std::ostream& os, TimePoint const& _)
  {
    return os << _.to("%c");
  }
  TimePoint() = default;
  template<typename C, typename D>
  TimePoint(std::chrono::time_point<C, D> const& _)
    : _(std::chrono::system_clock::to_time_t(_))
  {
  }

private:
  std::time_t _; // NOLINT
};
struct Timer final
{
  Timer() = default;

public:
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

  void start()
  {
    m_end = m_begin = std::chrono::system_clock::now();
  }
  void stop()
  {
    m_end = std::chrono::system_clock::now();
    m_duration = m_end - m_begin;
  }

  auto duration() const
  {
    return m_duration;
  }
  auto elapsed() const
  {
    return Duration(std::chrono::system_clock::now() - m_begin);
  }

  static auto current()
  {
    return TimePoint(std::chrono::system_clock::now());
  }

  template<typename Task>
  static auto time(Task&& task)
  {
    Timer _;
    _.start();
    task();
    _.stop();
    return _.duration();
  }

private:
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
  TimePoint m_begin, m_end;
  Duration  m_duration;
};

class ScopedTimer
{
  Timer                                  m_timer;
  std::function<void(const Duration& d)> m_callback;

public:
  using ResultCallback = std::function<void(const Duration& d)>;
  ScopedTimer(ResultCallback callback)
    : m_callback(std::move(callback))
  {
    m_timer.start();
  }
  ~ScopedTimer()
  {
    m_timer.stop();
    m_callback(m_timer.duration());
  }
};
} // namespace VGG::layer
