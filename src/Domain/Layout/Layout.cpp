#include "Layout.hpp"

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Utility/Log.hpp"
#include "Rule.hpp"

using namespace VGG;
using namespace VGG::Layout::Internal::Rule;

Layout::Layout::Layout(JsonDocumentPtr designDoc, JsonDocumentPtr layoutDoc, bool isRootTree)
  : m_designDoc{ designDoc }
  , m_layoutDoc{ layoutDoc }
  , m_isRootTree{ isRootTree }
{
  ASSERT(m_designDoc);

  if (m_layoutDoc)
  {
    collectRules(m_layoutDoc->content());
  }

  // initial config
  buildLayoutTree();
  configureNodeAutoLayout(m_layoutTree);
}

void Layout::Layout::layout(Size size)
{
  // todo, layout current page only

  // update root frame
  auto root = layoutTree();
  auto frame = root->frame();
  if (frame.size == size)
  {
    return;
  }

  frame.size = size;
  root->setFrame(frame);

  if (m_isRootTree)
  {
    // udpate page frame
    for (auto& page : root->children())
    {
      auto frame = page->frame();
      frame.size = size;
      page->setFrame(frame);
    }
  }

  // layout
  root->layoutIfNeeded();
}

void Layout::Layout::buildLayoutTree()
{
  auto& designJson = m_designDoc->content();
  if (!designJson.is_object())
  {
    WARN("invalid design file");
    return;
  }

  if (m_isRootTree)
  {
    m_layoutTree.reset(new LayoutNode{ "/", {} });
    json::json_pointer framesPath{ "/frames" };
    for (auto i = 0; i < designJson[K_FRAMES].size(); ++i)
    {
      auto path = framesPath / i;
      auto page = createOneLayoutNode(designJson[path], path, m_layoutTree);
      m_pageSize.push_back(page->frame().size);
    }
  }
  else
  {
    json::json_pointer path;
    m_layoutTree = createOneLayoutNode(designJson, path, nullptr);
  }
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
  node->setViewModel(m_designDoc);
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

  auto& designJson = m_designDoc->content();
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
