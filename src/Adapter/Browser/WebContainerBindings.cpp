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

#include "Browser/WebContainer.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

namespace VGG
{
namespace
{

EMSCRIPTEN_BINDINGS(vgg_container)
{
  using namespace emscripten;

  class_<WebContainer>("WebContainer")
    .constructor<int, int, float>()
    .function("load", &WebContainer::jsLoad)
    // on event: mouse/touch/wheel/key/resize
    .function("run", &WebContainer::jsRun);
}

} // namespace
} // namespace VGG