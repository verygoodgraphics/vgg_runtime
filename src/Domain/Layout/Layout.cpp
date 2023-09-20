#include "Layout.hpp"

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Utility/Log.hpp"
#include "Rule.hpp"

using namespace VGG;
using namespace VGG::Layout::Internal::Rule;

Layout::Layout::Layout(std::shared_ptr<Daruma> model)
  : m_model{ model }
{
  ASSERT(m_model);

  auto designJson = ExpandSymbol{ m_model->designDoc()->content() }();
  if (m_model->layoutDoc())
  {
    collectRules(m_model->layoutDoc()->content());
  }

  m_model->setRuntimeDesignDoc(designJson);

  // initial config
  auto document = layoutTree();
  configureNodeAutoLayout(document);
}

void Layout::Layout::layout(Size size)
{
  if (size == m_size)
  {
    return;
  }

  m_size = size;

  // update document frame
  auto document = layoutTree();
  auto frame = document->frame();
  frame.size = m_size;
  document->setFrame(frame);

  // udpate page frame
  for (auto& page : document->children())
  {
    auto frame = page->frame();
    frame.size = m_size;
    page->setFrame(frame);
  }

  // layout
  document->layoutIfNeeded();
}

std::shared_ptr<LayoutNode> Layout::Layout::layoutTree()
{
  if (m_layoutTree)
  {
    return m_layoutTree;
  }

  auto& designJson = m_model->runtimeDesignDoc()->content();
  if (!designJson.is_object())
  {
    WARN("invalid design file");
    return {};
  }

  m_layoutTree.reset(new LayoutNode{ "/", {} });

  json::json_pointer framesPath{ "/frames" };
  for (auto i = 0; i < designJson[K_FRAMES].size(); ++i)
  {
    auto path = framesPath / i;
    createOneLayoutNode(designJson[path], path, m_layoutTree);
  }

  m_size = m_layoutTree->frame().size;

  return m_layoutTree;
}

std::shared_ptr<LayoutNode> Layout::Layout::createOneLayoutNode(const nlohmann::json& j,
                                                                json::json_pointer currentPath,
                                                                std::shared_ptr<LayoutNode> parent)
{
  if (!j.is_object())
  {
    WARN("create one layout node from json object, json is not object, return");
    return nullptr;
  }

  if (!isLayoutNode(j))
  {
    return nullptr;
  }

  auto frame = j[K_FRAME].get<Rect>();
  frame.origin.y *= FLIP_Y_FACTOR;

  auto node = std::make_shared<LayoutNode>(currentPath.to_string(), frame);
  node->setViewModel(m_model->runtimeDesignDoc());
  if (parent)
  {
    parent->addChild(node);
  }

  for (auto& [key, val] : j.items())
  {
    auto path = currentPath;
    path /= key;

    createOneOrMoreLayoutNodes(val, path, node);
  }

  return node;
}

void Layout::Layout::createLayoutNodes(const nlohmann::json& j,
                                       json::json_pointer currentPath,
                                       std::shared_ptr<LayoutNode> parent)
{
  if (!j.is_array())
  {
    WARN("create layout nodes from json array, json is not array, return");
    return;
  }

  auto size = j.size();
  for (auto i = 0; i < size; ++i)
  {
    auto path = currentPath;
    path /= i;

    createOneOrMoreLayoutNodes(j[i], path, parent);
  }
}

void Layout::Layout::createOneOrMoreLayoutNodes(const nlohmann::json& j,
                                                json::json_pointer currentPath,
                                                std::shared_ptr<LayoutNode> parent)
{
  if (j.is_object())
  {
    createOneLayoutNode(j, currentPath, parent);
  }
  else if (j.is_array())
  {
    createLayoutNodes(j, currentPath, parent);
  }
}

void Layout::Layout::collectRules(const nlohmann::json& json)
{
  if (!json.is_object())
  {
    return;
  }

  auto& obj = json[K_OBJ];
  if (obj.is_array())
  {
    for (auto& item : obj)
    {
      auto& id = item[K_ID];
      auto rule = std::make_shared<Internal::Rule::Rule>();
      *rule = item;

      m_rules[id] = rule;
    }
  }
}

void Layout::Layout::configureNodeAutoLayout(std::shared_ptr<LayoutNode> node)
{
  std::shared_ptr<VGG::Layout::Internal::Rule::Rule> rule;

  auto& designJson = m_model->runtimeDesignDoc()->content();
  if (node->path() != "/")
  {
    auto& json = designJson[nlohmann::json::json_pointer{ node->path() }];
    if (json.contains(K_ID))
    {
      auto nodeId = json.at(K_ID).get<std::string>();
      if (m_rules.find(nodeId) != m_rules.end())
      {
        rule = m_rules[nodeId];
      }
    }
  }

  auto autoLayout = node->createAutoLayout();
  if (rule)
  {
    autoLayout->rule = rule;
    node->configureAutoLayout();
  }

  for (auto& child : node->children())
  {
    configureNodeAutoLayout(child);
  }
}
