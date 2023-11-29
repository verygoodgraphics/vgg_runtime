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
#include "Reporter.hpp"

#include "Domain/Daruma.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Domain/IVggEnv.hpp"
#include "Domain/Model/JsonKeys.hpp"
#include "Domain/VggExec.hpp"
#include "Utility/DIContainer.hpp"
#include "Utility/Log.hpp"

#include <sstream>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

// event key
constexpr auto K_TYPE = "type";

// event type
constexpr auto K_SELECT = "select";
constexpr auto K_FIRST_RENDER = "firstRender";

namespace
{
auto env()
{
  return VGG::DIContainer<std::shared_ptr<IVggEnv>>::get();
}
} // namespace

void Reporter::onSelectNode(std::weak_ptr<LayoutNode> node)
{
  auto target = node.lock();
  if (!target)
  {
    return;
  }

  auto model = DarumaContainer().get();
  ASSERT(model);
  if (!model)
  {
    return;
  }

  const auto& designDoc = model->runtimeDesignDoc();
  ASSERT(designDoc);
  if (!designDoc)
  {
    return;
  }

  nlohmann::json::json_pointer path{ target->path() };
  const auto&                  jsonNode = designDoc->content()[path];

  nlohmann::json event;
  event[K_TYPE] = K_SELECT;
  event[K_ID] = jsonNode[K_ID];
  event[K_PATH] = target->path();

  sendEventToJs(event);
}

void Reporter::onFirstRender()
{
  nlohmann::json event;
  event[K_TYPE] = K_FIRST_RENDER;

  sendEventToJs(event);
}

void Reporter::sendEventToJs(const nlohmann::json& event)
{
  DEBUG("Reporter::sendEventToJs, event is: %s", event.dump().c_str());

  auto jsEngine = m_jsEngine.lock();
  if (!jsEngine)
  {
    FAIL("#Reporter::sendEventToJs, no jsEngine");
    return;
  }

  std::ostringstream oss;
  oss << "(function (event) {"
      << "const containerKey = '" << env()->getContainerKey() << "';"
      << "const envKey = '" << env()->getEnv() << "';"
      << "const listenerKey = '" << env()->getListenerKey() << "';"
      << R"(
          const listener = globalThis[containerKey]?.[envKey]?.[listenerKey]
          if(listener) {
            listener(event);
          }
        })
      )"
      << "('" << event.dump() << "');";
  DEBUG("Reporter::sendEventToJs, script is: %s", oss.str().c_str());

  jsEngine->evalModule(oss.str());
}
