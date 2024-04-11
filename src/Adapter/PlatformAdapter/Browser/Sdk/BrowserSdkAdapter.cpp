/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Domain/Layout/Math.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;
using namespace VGG;
using BEvent = BrowserAdapter::Event;
using BUIEvent = BrowserAdapter::UIEvent;

using BKeyboardEvent = BrowserAdapter::KeyboardEvent;
using BMouseEvent = BrowserAdapter::MouseEvent;
using BTouchEvent = BrowserAdapter::TouchEvent;

EMSCRIPTEN_BINDINGS(vgg_sdk)
{
  value_object<ISdk::ImageOptions>("ImageOptions")
    .field("type", &ISdk::ImageOptions::type)
    .field("quality", &ISdk::ImageOptions::quality);

  class_<VggSdk>("VggSdk")
    .constructor<>()
    // env
    .function("getEnv", &VggSdk::getEnv)
    .function("setContainerKey", &VggSdk::setContainerKey)
    .function("setInstanceKey", &VggSdk::setInstanceKey)
    .function("setListenerKey", &VggSdk::setListenerKey)
    // misc
    .function("texts", &VggSdk::texts)
    .function("makeImageSnapshot", &VggSdk::emMakeImageSnapshot)
    // doc
    .function("getElement", &VggSdk::getElement)
    .function("updateElement", &VggSdk::updateElement)
    .function("getDesignDocument", &VggSdk::designDocument)
    .function("valueAt", &VggSdk::designDocumentValueAt)
    // frames
    .function("getFramesInfo", &VggSdk::getFramesInfo)
    .function("currentFrameId", &VggSdk::currentFrameId)
    .function("setCurrentFrameById", &VggSdk::setCurrentFrameById)
    .function("launchFrameId", &VggSdk::launchFrameId)
    .function("setLaunchFrameById", &VggSdk::setLaunchFrameById)
    .function("presentFrameById", &VggSdk::presentFrameById)
    .function("dismissFrame", &VggSdk::dismissFrame)
    // font
    .function("requiredFonts", &VggSdk::requiredFonts)
    .function("addFont", &VggSdk::jsAddFont)
    // event listener
    .function("addEventListener", &VggSdk::addEventListener)
    .function("removeEventListener", &VggSdk::removeEventListener)
    .function("getEventListeners", &VggSdk::getEventListeners)
    // editor
    .function("vggFileUint8Array", &VggSdk::vggFileUint8Array);
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

EMSCRIPTEN_BINDINGS(vgg_matrix)
{
  using Matrix = Layout::Matrix;

  class_<Matrix>("Matrix")
    .constructor<double, double, double, double, double, double>()
    .property("a", &Matrix::a)
    .property("b", &Matrix::b)
    .property("c", &Matrix::c)
    .property("d", &Matrix::d)
    .property("tx", &Matrix::tx)
    .property("ty", &Matrix::ty)
    .function("rotationAngle", &Matrix::decomposeRotateRadian)
    .function("rotated", &Matrix::rotated)
    .function("scaled", &Matrix::scaled);
}

// https://github.com/emscripten-core/emscripten/issues/11070
namespace emscripten
{
namespace internal
{

template<typename T, typename Allocator>
struct BindingType<std::vector<T, Allocator>>
{
  using ValBinding = BindingType<val>;
  using WireType = ValBinding::WireType;

  static WireType toWireType(const std::vector<T, Allocator>& vec)
  {
    std::vector<val> valVec(vec.begin(), vec.end());
    return BindingType<val>::toWireType(val::array(valVec));
  }

  static std::vector<T, Allocator> fromWireType(WireType value)
  {
    return vecFromJSArray<T>(ValBinding::fromWireType(value));
  }
};

template<typename T>
struct TypeID<
  T,
  typename std::enable_if_t<std::is_same<
    typename Canonicalized<T>::type,
    std::vector<
      typename Canonicalized<T>::type::value_type,
      typename Canonicalized<T>::type::allocator_type>>::value>>
{
  static constexpr TYPEID get()
  {
    return TypeID<val>::get();
  }
};

} // namespace internal
} // namespace emscripten