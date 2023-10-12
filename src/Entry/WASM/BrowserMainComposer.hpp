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
#pragma once

#include "Entry/SDL/SdlMouse.hpp"

#include "Adapter/BrowserComposer.hpp"
#include "Application/MainComposer.hpp"

class VggBrowser
{
public:
  VggBrowser() = delete;

  static VGG::MainComposer& mainComposer()
  {
    static VGG::MainComposer s_instance{ new BrowserComposer(), std::make_shared<SdlMouse>() };
    return s_instance;
  }
};
