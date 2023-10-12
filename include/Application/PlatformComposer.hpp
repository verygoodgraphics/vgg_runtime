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

#include "VggEnv.hpp"
#include "Domain/VggExec.hpp"
#include "Domain/VggJSEngine.hpp"
#include "Utility/DIContainer.hpp"

class PlatformComposer
{
public:
  virtual ~PlatformComposer() = default;

  auto setup()
  {
    auto jsEngine = std::make_shared<VggExec>(createJsEngine(), std::make_shared<VggEnv>());
    VGG::DIContainer<std::shared_ptr<VggExec>>::get() = jsEngine;

    platformSetup();

    return jsEngine;
  }

  void teardown()
  {
    VGG::DIContainer<std::shared_ptr<VggExec>>::get().reset();
    platformTeardown();
  }

  virtual std::shared_ptr<VggJSEngine> createJsEngine() = 0;

  virtual void platformSetup()
  {
  }
  virtual void platformTeardown()
  {
  }
};
