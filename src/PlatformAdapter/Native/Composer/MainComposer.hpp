#pragma once

#include "Api/VggEnv.hpp"
#include "Exec/VggExec.hpp"
#include "Utils/DIContainer.hpp"

class MainComposer
{
  bool m_catchJsException;

public:
  MainComposer(bool catchJsException = true)
    : m_catchJsException{ catchJsException }
  {
  }
  void setup(const std::string& sdkUrl);
  void teardown();

private:
  void setupVgg(std::shared_ptr<VggExec> vggExec, const std::string& sdkUrl);
};
