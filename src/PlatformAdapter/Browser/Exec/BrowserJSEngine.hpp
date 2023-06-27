#pragma once

#include "Exec/VggJSEngine.hpp"

#include <string>

class BrowserJSEngine final : public VggJSEngine
{
public:
  bool evalScript(const std::string& code);
  bool evalModule(const std::string& code);

  bool evalModule(const std::string& code, VGG::EventPtr event);

private:
  std::string m_moduleWrapper;
  int m_evalTimes = 0;
};
