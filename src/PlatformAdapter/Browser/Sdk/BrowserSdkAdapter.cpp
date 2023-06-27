#include "MouseEventWrapper.hpp"

#include "Sdk/VggSdk.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace VGG;

EMSCRIPTEN_BINDINGS(vgg_sdk)
{
  class_<VggSdk>("VggSdk")
    .constructor<>()
    .function("getDesignDocument", &VggSdk::designDocument)
    .function("addAt", &VggSdk::designDocumentAddAt)
    .function("deleteAt", &VggSdk::designDocumentDeleteAt)
    .function("updateAt", &VggSdk::designDocumentReplaceAt)
    .function("addEventListener", &VggSdk::addEventListener)
    .function("removeEventListener", &VggSdk::removeEventListener)
    .function("getEventListeners", &VggSdk::getEventListeners);
}

EMSCRIPTEN_BINDINGS(vgg_event)
{
  class_<MouseEventWrapper>("VggMouseEvent")
    .constructor<>()
    .property("button", &MouseEventWrapper::button)
    .function("preventDefault", &MouseEventWrapper::preventDefault)
    .function("bindCppEvent", &MouseEventWrapper::bindCppEvent);
}