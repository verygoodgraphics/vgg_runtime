#ifndef BROWSER_JS_ENGINE_HPP
#define BROWSER_JS_ENGINE_HPP

#include "VggJSEngine.hpp"

class BrowserJSEngine : public VggJSEngine {
public:
  bool eval(std::string_view buffer, const char *filename, int flags);
};

#endif