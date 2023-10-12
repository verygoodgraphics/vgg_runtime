/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include <rxcpp/rx.hpp>

namespace VGG
{

class RunLoop
{
  rxcpp::schedulers::run_loop m_runLoop;

public:
  rxcpp::observe_on_one_worker thread()
  {
    return rxcpp::observe_on_run_loop(m_runLoop);
  }

  void dispatch()
  {
    while (!m_runLoop.empty() && m_runLoop.peek().when < m_runLoop.now())
    {
      m_runLoop.dispatch();
    }
  }
};

}
