#include "KeyboardEvent.hpp"
#include "MouseEvent.hpp"
#include "TouchEvent.hpp"

#include "Sdk/VggSdk.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace VGG;
using BEvent = BrowserAdapter::Event;
using BUIEvent = BrowserAdapter::UIEvent;

using BKeyboardEvent = BrowserAdapter::KeyboardEvent;
using BMouseEvent = BrowserAdapter::MouseEvent;
using BTouchEvent = BrowserAdapter::TouchEvent;

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
  // base
  class_<BEvent>("VggEvent")
    .constructor<>()
    .property("target", &BEvent::target)
    .property("type", &BEvent::type)
    .function("preventDefault", &BEvent::preventDefault)
    .function("bindCppEvent", &BEvent::bindCppEvent);

  // middle
  class_<BUIEvent, base<BEvent>>("VggUIEvent").constructor<>();

  // child
  class_<BKeyboardEvent, base<BUIEvent>>("VggKeyboardEvent")
    .constructor<>()
    .property("key", &BKeyboardEvent::key);

  class_<BMouseEvent, base<BUIEvent>>("VggMouseEvent")
    .constructor<>()
    .property("altkey", &BMouseEvent::altkey)
    .property("button", &BMouseEvent::button)
    .property("clientX", &BMouseEvent::x)
    .property("clientY", &BMouseEvent::y)
    .property("ctrlkey", &BMouseEvent::ctrlkey)
    .property("metakey", &BMouseEvent::metakey)
    .property("movementX", &BMouseEvent::movementX)
    .property("movementY", &BMouseEvent::movementY)
    .property("shiftkey", &BMouseEvent::shiftkey)
    .property("x", &BMouseEvent::x)
    .property("y", &BMouseEvent::y);

  class_<BTouchEvent, base<BUIEvent>>("VggTouchEvent").constructor<>();
}