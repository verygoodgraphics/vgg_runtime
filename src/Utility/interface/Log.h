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
#ifndef __LOG__H_
#define __LOG__H_

// #define LOG_NODEBUG
// #define LOG_NOASSERT
// #define LOG_NOPROFILE
// #define LOG_LOGFILE "log.txt

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <memory>

/**************************************************************************************************/
/* Platform                                                                                       */
/**************************************************************************************************/
#ifdef __APPLE__
#define MOD4 KMOD_GUI
#else
#define MOD4 KMOD_CTRL
#endif

/**************************************************************************************************/
/* Common                                                                                         */
/**************************************************************************************************/
/// STR(X) turns X into a string literature
/// XSTR(X) turns X into string of expansion of macro X
#define STR(X) #X
#define XSTR(X) STR(X)

/// Supress unused warning
#define UNUSED(X) (void)(X)

#define STRFMT(fmt, ...)                                                                           \
  (                                                                                                \
    [&]()                                                                                          \
    {                                                                                              \
      int sz = snprintf(nullptr, 0, fmt, __VA_ARGS__);                                             \
      assert(sz >= 0);                                                                             \
      std::unique_ptr<char> buf{ new char[sz + 1] };                                               \
      snprintf(buf.get(), sz + 1, fmt, __VA_ARGS__);                                               \
      return std::string(buf.get());                                                               \
    })()

template<typename... Args>
inline std::string strfmt(const char* fmt, Args... args)
{
  int sz = snprintf(nullptr, 0, fmt, args...);
  assert(sz >= 0);
  std::unique_ptr<char> buf{ new char[sz + 1] };
  snprintf(buf.get(), sz + 1, fmt, args...);
  return std::string(buf.get());
}

inline std::string strlimit(const std::string& s,
                            size_t max = 10,
                            const std::string& postfix = "...")
{
  std::string ls{ s };
  if (s.size() > max)
  {
    ls = s.substr(0, max) + postfix;
  }
  return ls;
}

/// Arithmetics
#define SQR(X) ((X) * (X))
#define FLOAT2SINT(X) (1e3 * (X))
#define SINT2FLOAT(X) ((X) / 1e3)

inline unsigned char getNthFractionalDigit(size_t n, double v)
{
  double coef = std::pow(10, n);
  int m = std::fabs(coef * v);
  return m % 10;
}

inline std::string scalarfmt(double v, size_t maxFractionalDigitNums = 2)
{
  size_t n = maxFractionalDigitNums;
  double rv = v;
  while (true)
  {
    while (n > 0)
    {
      if (getNthFractionalDigit(n, rv) == 0)
      {
        n -= 1;
      }
      else
      {
        break;
      }
    }
    rv = std::round(rv * std::pow(10, n)) / std::pow(10, n);
    if (getNthFractionalDigit(n, rv) != 0 || n == 0)
    {
      break;
    }
  }
  return strfmt("%.*lf", n, rv);
}

/**************************************************************************************************/
/* Logging                                                                                        */
/**************************************************************************************************/
/// General Formatting
#define _FMT_RESET_ "0"
#define _FMT_BRIGHT_ "1"
#define _FMT_DIM_ "2"
#define _FMT_UNDERSCORE_ "3"
#define _FMT_BLINK_ "4"
#define _FMT_REVERSE_ "5"
#define _FMT_HIDDEN_ "6"

/// Foreground Colors
#define _FG_BLACK_ "30"
#define _FG_RED_ "31"
#define _FG_GREEN_ "32"
#define _FG_YELLOW_ "33"
#define _FG_BLUE_ "34"
#define _FG_MAGENTA_ "35"
#define _FG_CYAN_ "36"
#define _FG_WHITE_ "37"

/// Background Colors
#define _BG_BLACK_ "40"
#define _BG_RED_ "41"
#define _BG_GREEN_ "42"
#define _BG_YELLOW_ "43"
#define _BG_BLUE_ "44"
#define _BG_MAGENTA_ "45"
#define _BG_CYAN_ "46"
#define _BG_WHITE_ "47"

/// Common formatters
#ifndef EMSCRIPTEN
#define FAIL_COLOR(X) "\x1b[" _FMT_BRIGHT_ ";" _FG_RED_ "m" X "\x1b[" _FMT_RESET_ "m"
#define WARN_COLOR(X) "\x1b[" _FMT_BRIGHT_ ";" _FG_YELLOW_ "m" X "\x1b[" _FMT_RESET_ "m"
#define INFO_COLOR(X) "\x1b[" _FMT_BRIGHT_ ";" _FG_BLUE_ "m" X "\x1b[" _FMT_RESET_ "m"
#else
#define FAIL_COLOR(X) X
#define WARN_COLOR(X) X
#define INFO_COLOR(X) X
#endif

/// Some variables
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/// Log file
namespace LOG
{
static inline FILE* _log_file_()
{
#ifdef LOG_LOGFILE
  static FILE* fp = fopen(LOG_LOGFILE, "a+");
  if (fp)
  {
    return fp;
  }
  else
  {
    return stderr;
  }
#else
#ifndef EMSCRIPTEN
  return stderr;
#else
  return stdout;
#endif
#endif
}
} // namespace Log

/// Private logging macros
#define _MSG_(type, log, msg, ...)                                                                 \
  do                                                                                               \
  {                                                                                                \
    fprintf((log), "[" type "] " msg "\n", ##__VA_ARGS__);                                         \
    fflush((log));                                                                                 \
  } while (0)

/// Public logging macros
#define FAIL(msg, ...)                                                                             \
  do                                                                                               \
  {                                                                                                \
    _MSG_("FAIL", LOG::_log_file_(), FAIL_COLOR(msg), ##__VA_ARGS__);                            \
  } while (0)
#define FAILED(msg, ...)                                                                           \
  do                                                                                               \
  {                                                                                                \
    _MSG_("FAIL", LOG::_log_file_(), FAIL_COLOR("%s:%d "), __FILENAME__, __LINE__);              \
    FAIL(msg, ##__VA_ARGS__);                                                                      \
    exit(EXIT_FAILURE);                                                                            \
  } while (0)
#define WARN(msg, ...)                                                                             \
  do                                                                                               \
  {                                                                                                \
    _MSG_("WARN", LOG::_log_file_(), WARN_COLOR(msg), ##__VA_ARGS__);                            \
  } while (0)
#define WARNL(msg, ...)                                                                            \
  do                                                                                               \
  {                                                                                                \
    _MSG_("WARN",                                                                                  \
          LOG::_log_file_(),                                                                     \
          WARN_COLOR("(%s:%d) " msg),                                                              \
          __FILENAME__,                                                                            \
          __LINE__,                                                                                \
          ##__VA_ARGS__);                                                                          \
  } while (0)
#define INFO(msg, ...)                                                                             \
  do                                                                                               \
  {                                                                                                \
    _MSG_("INFO", LOG::_log_file_(), INFO_COLOR(msg), ##__VA_ARGS__);                            \
  } while (0)
#ifndef LOG_NODEBUG
#define DEBUG(msg, ...)                                                                            \
  do                                                                                               \
  {                                                                                                \
    _MSG_("DEBUG", LOG::_log_file_(), msg, ##__VA_ARGS__);                                       \
  } while (0)
#else
#define DEBUG(msg, ...)
#endif

/**************************************************************************************************/
/* Assertion                                                                                      */
/**************************************************************************************************/
/// Private assert macros
#ifndef LOG_NOASSERT
#define _ASSERT_(x, msg, ...)                                                                      \
  do                                                                                               \
  {                                                                                                \
    if (!(x))                                                                                      \
    {                                                                                              \
      if (strcmp((msg), ""))                                                                       \
      {                                                                                            \
        FAILED(STR(x) " : " msg, ##__VA_ARGS__);                                                   \
      }                                                                                            \
      else                                                                                         \
      {                                                                                            \
        FAILED(STR(x));                                                                            \
      }                                                                                            \
    }                                                                                              \
  } while (0)
#define _EXEC_(x, cb)                                                                              \
  do                                                                                               \
  {                                                                                                \
    if (!(x))                                                                                      \
    {                                                                                              \
      cb();                                                                                        \
    }                                                                                              \
  } while (0)
#else
#define _ASSERT_(x, msg, ...)                                                                      \
  do                                                                                               \
  {                                                                                                \
    UNUSED(x);                                                                                     \
  } while (0)
#define _EXEC_(x, cb)
#endif

/// Public assert macros
#define ASSERT(x)                                                                                  \
  do                                                                                               \
  {                                                                                                \
    _ASSERT_(x, "");                                                                               \
  } while (0)
#define ASSERT_MSG(x, msg, ...)                                                                    \
  do                                                                                               \
  {                                                                                                \
    _ASSERT_(x, FAIL_COLOR(msg), ##__VA_ARGS__);                                                   \
  } while (0)
#define ASSERT_CB(x, cb, msg, ...)                                                                 \
  do                                                                                               \
  {                                                                                                \
    if (!(x))                                                                                      \
    {                                                                                              \
      _EXEC_(1, cb);                                                                               \
      _ASSERT_(1, FAIL_COLOR(msg), ##__VA_ARGS__);                                                 \
    }                                                                                              \
  } while (0)

/**************************************************************************************************/
/* Profile                                                                                        */
/**************************************************************************************************/
#ifndef LOG_NOPROFILE
#include <chrono>
#include <unordered_map>
namespace LOG
{
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using TimeMap = std::unordered_map<std::string, TimePoint>;
using Precision = std::chrono::milliseconds;

static TimeMap _timer_map_;
static int _padding_counter_{ 1 };

static inline void _timer_start_(const std::string& id)
{
  _padding_counter_ += 2;
  _timer_map_[id] = Clock::now();
}

static inline double _timer_stop_(const std::string& id)
{
  _padding_counter_ = (_padding_counter_ > 3) ? (_padding_counter_ - 2) : 1;
  TimeMap::iterator it = _timer_map_.find(id);
  if (it == _timer_map_.end())
  {
    return -1;
  }
  const TimePoint now = Clock::now();
  return std::chrono::duration_cast<Precision>(now - it->second).count();
}
} // namespace LOG
#define PROFILE(id, ...)                                                                           \
  do                                                                                               \
  {                                                                                                \
    _MSG_("PROFILE >>>",                                                                           \
          LOG::_log_file_(),                                                                     \
          "%*s----- %s",                                                                           \
          LOG::_padding_counter_,                                                                \
          "{",                                                                                     \
          id,                                                                                      \
          ##__VA_ARGS__);                                                                          \
    LOG::_timer_start_(id);                                                                      \
  } while (0)
#define PROFILE_END(id, ...)                                                                       \
  do                                                                                               \
  {                                                                                                \
    const double c = LOG::_timer_stop_(id);                                                      \
    if (c >= 0)                                                                                    \
    {                                                                                              \
      _MSG_("*TIME COST*",                                                                         \
            LOG::_log_file_(),                                                                   \
            "%*s%.3lfs",                                                                           \
            LOG::_padding_counter_ - 1,                                                          \
            "",                                                                                    \
            c / 1000);                                                                             \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
      WARN("%s:%d NO PROFILE START FOUND", __FILENAME__, __LINE__);                                \
    }                                                                                              \
    _MSG_("PROFILE END",                                                                           \
          LOG::_log_file_(),                                                                     \
          "%*s----} %s",                                                                           \
          LOG::_padding_counter_,                                                                \
          "-",                                                                                     \
          id,                                                                                      \
          ##__VA_ARGS__);                                                                          \
  } while (0)
#else
#define PROFILE(id, ...)
#define PROFILE_END(id, ...)
#endif

#endif //__LOG__H_
