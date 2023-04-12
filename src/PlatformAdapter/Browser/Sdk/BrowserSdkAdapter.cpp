#include "VggSdk.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(vgg_sdk) {
  class_<VggSdk>("VggSdk")
      .constructor<>()
      .function("updateStyle", &VggSdk::updateStyle)
      .function("getDocumentJson", &VggSdk::documentJson)
      .function("jsonAt", &VggSdk::jsonAt)
      .function("replaceInDocument", &VggSdk::replaceInDocument);
}