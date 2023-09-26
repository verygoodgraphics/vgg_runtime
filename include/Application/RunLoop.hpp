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