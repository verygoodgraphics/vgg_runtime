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

#include "VggTimer.hpp"
#include <stdint.h>
#include <chrono>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#include "Application/RunLoop.hpp"
#include <rxcpp/rx-includes.hpp>
#include <rxcpp/operators/rx-lift.hpp>
#include <rxcpp/operators/rx-observe_on.hpp>
#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-operators.hpp>
#include <rxcpp/rx-predef.hpp>
#include <rxcpp/rx-sources.hpp>
#include <rxcpp/sources/rx-interval.hpp>
#include <rxcpp/sources/rx-timer.hpp>

namespace VGG
{

Timer::Timer(double interval, TCallback callback, bool repeats)
  : m_callback(callback)
{
  auto period = std::chrono::milliseconds(static_cast<int64_t>(interval * 1000));

  if (repeats)
  {
    auto values = rxcpp::observable<>::interval(period);
    m_timer = values.observe_on(RunLoop::sharedInstance()->thread())
                .subscribe([this](int v) { this->m_callback(); }, []() {});
  }
  else
  {
    auto values = rxcpp::observable<>::timer(period);
    m_timer = values.observe_on(RunLoop::sharedInstance()->thread())
                .subscribe(
                  [this](int v)
                  {
                    if (this->m_callback)
                    {
                      this->m_callback();
                    }
                  },
                  []() {});
  }
}

void Timer::invalidate()
{
  m_timer.unsubscribe();
}

} // namespace VGG