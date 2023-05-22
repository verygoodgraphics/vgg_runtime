#pragma once

#include "rxcpp/rx.hpp"

namespace VGG
{

class RunLoop
{
  rxcpp::schedulers::run_loop m_run_loop;

public:
  rxcpp::observe_on_one_worker thread()
  {
    return rxcpp::observe_on_run_loop(m_run_loop);
  }

  void dispatch()
  {
    while (!m_run_loop.empty())
    {
      m_run_loop.dispatch();
    }
  }
};

}