#pragma once

#include "NativeExec.hpp"
#include "VggEnv.hpp"
#include "VggDepContainer.hpp"
#include "VggExec.hpp"

class MainComposer
{
public:
  void setup(const std::string& sdkUrl);
  void teardown();

private:
  void setupVgg(std::shared_ptr<VggExec> vggExec, const std::string& sdkUrl);
};
