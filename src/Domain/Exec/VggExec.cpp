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
#include "VggExec.hpp"

#include <sstream>

using namespace VGG;

bool VggExec::evalScript(const std::string& program)
{
  setEnv();
  return m_jsEngine->evalScript(program);
}

bool VggExec::evalModule(const std::string& program)
{
  setEnv();
  return m_jsEngine->evalModule(program);
}

bool VggExec::evalModule(const std::string& code, VGG::EventPtr event)
{
  setEnv();
  return m_jsEngine->evalModule(code, event, m_env);
}

void VggExec::setEnv()
{
  std::ostringstream oss;
  oss << "(function () {"
      << "const containersKey = '" << m_env->getContainerKey() << "';"
      << "const envName = '" << m_env->getEnv() << "';"
      << "const instanceKey = '" << m_env->getInstanceKey() << "';"
      << "const currentVggEnvName = '" << m_env->currrentEnvName() << "';"
      << "const currentVggName = '" << m_env->currrentVggName() << "';"
      << R"(
          globalThis[currentVggEnvName] = envName;
          globalThis[currentVggName] = globalThis[containersKey]?.[envName]?.[instanceKey];
        })();
      )";
  m_jsEngine->evalScript(oss.str());
}
