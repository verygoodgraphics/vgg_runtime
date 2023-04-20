#ifndef VGG_JS_ENGINE_HPP
#define VGG_JS_ENGINE_HPP

#include <string_view>

class VggJSEngine
{
public:
  virtual ~VggJSEngine() = default;

  virtual bool evalScript(const std::string& code) = 0;
  virtual bool evalModule(const std::string& code) = 0;
};

#endif