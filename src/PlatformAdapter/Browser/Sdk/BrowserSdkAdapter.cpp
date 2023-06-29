#include "MouseEvent.hpp"

#include "Sdk/VggSdk.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace VGG;
using BEvent = BrowserAdapter::Event;
using BUIEvent = BrowserAdapter::UIEvent;
using BMouseEvent = BrowserAdapter::MouseEvent;

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
  class_<BEvent>("VggEvent")
    .constructor<>()
    .property("target", &BEvent::target)
    .property("type", &BEvent::type)
    .function("preventDefault", &BEvent::preventDefault)
    .function("bindCppEvent", &BEvent::bindCppEvent);

  class_<BUIEvent, base<BEvent>>("VggUIEvent").constructor<>();

  class_<BMouseEvent, base<BUIEvent>>("VggMouseEvent")
    .constructor<>()
    .property("button", &BMouseEvent::button);
}