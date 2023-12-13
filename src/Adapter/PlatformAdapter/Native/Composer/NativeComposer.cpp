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

#include "NativeComposer.hpp"

#include "Domain/IVggEnv.hpp"
#include "Utility/Log.hpp"

using namespace VGG;

void NativeComposer::setupVgg()
{
  auto env = m_env.lock();
  ASSERT(env);

  std::ostringstream oss;
  oss << "(function () {"
      << "const containerKey = '" << env->getContainerKey() << "';"
      << "const envKey = '" << env->getEnv() << "';"
      << "const instanceKey = '" << env->getInstanceKey() << "';"
      << R"(
          var vggSdkAddon = process._linkedBinding('vgg_sdk_addon');

          globalThis[containerKey] ??= {};
          globalThis[containerKey][envKey] ??= {};
          globalThis[containerKey][envKey][instanceKey] = vggSdkAddon;
        })();
      )";
  m_native_exec->evalModule(oss.str());

  if (m_catchJsException)
  {
    std::string catch_exception{ R"(
        const __vggErrorHandler = (err) => {
          console.error(err)
        }
        process.on('uncaughtException', __vggErrorHandler)
      )" };
    m_native_exec->evalScript(catch_exception);
  }
}