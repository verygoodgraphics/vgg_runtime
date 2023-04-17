#pragma once

#include "NativeExec.hpp"
#include "VggEnv.hpp"
#include "VggDepContainer.hpp"
#include "VggExec.hpp"

class MainComposer
{
public:
  void setup();
  void teardown();

private:
  void setupVgg(std::shared_ptr<VggExec> vggExec);
};
