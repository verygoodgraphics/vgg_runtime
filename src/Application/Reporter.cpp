#include "Reporter.hpp"

#include "Domain/Daruma.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Domain/Model/JsonKeys.hpp"
#include "Domain/VggExec.hpp"
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
  const auto& jsonNode = designDoc->content()[path];

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

  auto dicUrl = "https://s5.vgg.cool/vgg-di-container.esm.js";
  std::ostringstream oss;
  oss << R"((async function (event) {
      let container = await import(')"
      << dicUrl << "');"
      << R"(
      const key = 'vggListener';
      const listener = container.vggGetObject(key); 
      if(listener) {
        listener(event);
      }
    })(')"
      << event.dump() << "');";

  DEBUG("Reporter::sendEventToJs, script is: %s", oss.str().c_str());

  jsEngine->evalModule(oss.str());
}