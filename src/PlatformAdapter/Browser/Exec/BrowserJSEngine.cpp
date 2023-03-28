#include "BrowserJSEngine.hpp"

#include <emscripten/emscripten.h>

bool BrowserJSEngine::eval(std::string_view buffer, const char *filename,
                           int flags) {
  // make sure buffer is null terminated string
  emscripten_run_script(buffer.data());

  return true;
}