#pragma once

#include "Exec/VggJSEngine.hpp"

#include <string>

class BrowserJSEngine : public VggJSEngine
{
public:
  bool evalScript(const std::string& code);
  bool evalModule(const std::string& code);

private:
  std::string m_moduleWrapper;
  int m_evalTimes = 0;
};
