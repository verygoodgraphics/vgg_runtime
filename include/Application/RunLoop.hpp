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
#pragma once

#include <chrono>
#include <compare>
#include <memory>
#include <rxcpp/operators/rx-observe_on.hpp>
#include <rxcpp/schedulers/rx-runloop.hpp>

#if defined(__ANDROID__)
#include <android/looper.h>
#endif

#include "Utility/Log.hpp"

namespace VGG
{

class RunLoop
{
  rxcpp::schedulers::run_loop m_runLoop;

public:
  static std::shared_ptr<RunLoop> sharedInstance();

  rxcpp::observe_on_one_worker thread()
  {
#if defined(__ANDROID__)
    ALooper* looper = ALooper_forThread();
    if (looper == nullptr) {
      looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }
    return rxcpp::observe_on_new_thread();
#else
    return rxcpp::observe_on_run_loop(m_runLoop);
#endif
  }

  bool empty() const
  {
    return m_runLoop.empty();
  }

  void dispatch()
  {
    while (!m_runLoop.empty() && m_runLoop.peek().when < m_runLoop.now())
    {
      m_runLoop.dispatch();
    }
  }

private:
  RunLoop() = default;
};

} // namespace VGG
