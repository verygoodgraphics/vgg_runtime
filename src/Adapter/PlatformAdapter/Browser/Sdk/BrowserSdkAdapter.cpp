/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "KeyboardEvent.hpp"
#include "MouseEvent.hpp"
#include "TouchEvent.hpp"

#include "Application/VggSdk.hpp"

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
    // env
    .function("getEnvKey", &VggSdk::getEnvKey)
    .function("setContainerKey", &VggSdk::setContainerKey)
    .function("setInstanceKey", &VggSdk::setInstanceKey)
    .function("setListenerKey", &VggSdk::setListenerKey)
    // doc
    .function("getDesignDocument", &VggSdk::designDocument)
    .function("addAt", &VggSdk::designDocumentAddAt)
    .function("deleteAt", &VggSdk::designDocumentDeleteAt)
    .function("updateAt", &VggSdk::designDocumentReplaceAt)
    // event listener
    .function("addEventListener", &VggSdk::addEventListener)
    .function("removeEventListener", &VggSdk::removeEventListener)
    .function("getEventListeners", &VggSdk::getEventListeners)
    // editor
    .function("save", &VggSdk::save);
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
    .property("key", &BKeyboardEvent::key)
    .property("repeat", &BKeyboardEvent::repeat)
    .property("altkey", &BKeyboardEvent::altkey)
    .property("ctrlkey", &BKeyboardEvent::ctrlkey)
    .property("metakey", &BKeyboardEvent::metakey)
    .property("shiftkey", &BKeyboardEvent::shiftkey);

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
