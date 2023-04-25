#pragma once

#include "Api/VggEnv.hpp"
#include "Exec/VggExec.hpp"
#include "Utils/DIContainer.hpp"

class MainComposer
{
public:
  void setup(const std::string& sdkUrl);
  void teardown();

private:
  void setupVgg(std::shared_ptr<VggExec> vggExec, const std::string& sdkUrl);
};
