#ifndef BROWSER_JS_ENGINE_HPP
#define BROWSER_JS_ENGINE_HPP

#include "VggJSEngine.hpp"

#include <string>

class BrowserJSEngine : public VggJSEngine
{
public:
  bool evalScript(const std::string& code);
  bool evalModule(const std::string& code);

private:
  std::string m_moduleWrapper;
  std::string url_encode(const std::string& value);
};

#endif