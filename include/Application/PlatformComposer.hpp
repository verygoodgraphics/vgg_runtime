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
#pragma once

#include "Application/VggEnv.hpp"
#include "Domain/VggExec.hpp"
#include "Domain/VggJSEngine.hpp"

namespace VGG
{

class PlatformComposer
{
protected:
  std::weak_ptr<VggEnv> m_env;

public:
  virtual ~PlatformComposer() = default;

  auto setup(std::shared_ptr<VggEnv> env)
  {
    m_env = env;

    auto jsEngine = std::make_shared<VggExec>(createJsEngine(), env);

    platformSetup();

    return jsEngine;
  }

  void teardown()
  {
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

} // namespace VGG