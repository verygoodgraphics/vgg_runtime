#ifndef VGG_JS_ENGINE_HPP
#define VGG_JS_ENGINE_HPP

#include <string_view>


class VggJSEngine {
public:
  virtual ~VggJSEngine() = default;
   
  virtual bool eval(std::string_view buffer, const char *filename = "<eval>",
            int flags = 0) = 0;
};

#endif