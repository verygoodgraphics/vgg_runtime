#pragma once

#include "Domain/VggJSEngine.hpp"

#include <string>

class BrowserJSEngine final : public VggJSEngine
{
public:
  bool evalScript(const std::string& code) override;
  bool evalModule(const std::string& code) override;
  bool evalModule(const std::string& code, VGG::EventPtr event) override;

private:
  std::string m_moduleWrapper;
  int m_evalTimes = 0;
};
