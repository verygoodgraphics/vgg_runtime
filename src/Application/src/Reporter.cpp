#include "Reporter.hpp"

#include <Domain/Daruma.hpp>
#include <Domain/DarumaContainer.hpp>
#include <Domain/Model/JsonKeys.hpp>

using namespace VGG;

void Reporter::onSelectNode(std::weak_ptr<LayoutNode> node)
{
  auto target = node.lock();
  if (!target)
  {
    return;
  }

  auto model = DarumaContainer().get();
  if (!model)
  {
    return;
  }

  const auto& designDoc = model->runtimeDesignDoc();
  if (!designDoc)
  {
    return;
  }

  nlohmann::json::json_pointer path{ target->path() };
  const auto& jsonNode = designDoc->content()[path];

  nlohmann::json event;
  event["from"] = "Editor";
  event["type"] = "selectNode";
  event["targetId"] = jsonNode[K_ID];

  sendEventToJs(event);
}

void Reporter::sendEventToJs(const nlohmann::json& event)
{
}