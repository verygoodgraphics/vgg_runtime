#include "Sdk/VggSdk.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(vgg_sdk)
{
  class_<VggSdk>("VggSdk")
    .constructor<>()
    .function("getDesignDocument", &VggSdk::designDocument)
    .function("addAt", &VggSdk::designDocumentAddAt)
    .function("deleteAt", &VggSdk::designDocumentDeleteAt)
    .function("updateAt", &VggSdk::designDocumentReplaceAt);
}