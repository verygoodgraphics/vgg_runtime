#include "Layout.hpp"

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"
#include "Rule.hpp"

using namespace VGG;
using namespace VGG::Layout::Internal::Rule;

constexpr auto FLIP_Y_FACTOR = -1;

void Layout::Layout::layout(Size size)
{
  auto root = layoutTree();

  if (size != m_size)
  {
    m_size = size;

    // initial config
    configureAutoLayout(root);

    // udpate Frames's frame
    auto frame = root->frame();
    frame.size = m_size;
    root->setFrame(frame);
    for (auto& subview : root->children())
    {
      auto frame = subview->frame();
      frame.size = m_size;
      subview->setFrame(frame);
    }

    //
    root->layoutIfNeeded();
  }
}

std::shared_ptr<LayoutView> Layout::Layout::layoutTree()
{
  if (m_layoutTree)
  {
    return m_layoutTree;
  }

  if (!m_designJson.is_object())
  {
    WARN("invalid design file");
    return {};
  }

  m_layoutTree.reset(new LayoutView{ "/", {} });

  json::json_pointer framesPath{ "/frames" };
  for (auto i = 0; i < m_designJson[K_FRAMES].size(); ++i)
  {
    auto path = framesPath / i;
    createOneLayoutView(m_designJson[path], path, m_layoutTree);
  }

  m_size = m_layoutTree->frame().size;

  return m_layoutTree;
}

std::shared_ptr<LayoutView> Layout::Layout::createOneLayoutView(const nlohmann::json& j,
                                                                json::json_pointer currentPath,
                                                                std::shared_ptr<LayoutView> parent)
{
  if (!j.is_object())
  {
    WARN("create one layout view from json object, json is not object, return");
    return nullptr;
  }

  if (!isLayoutNode(j))
  {
    return nullptr;
  }

  auto frame = j[K_FRAME].get<Rect>();
  frame.origin.y *= FLIP_Y_FACTOR;

  auto layoutView = std::make_shared<LayoutView>(currentPath.to_string(), frame);
  if (parent)
  {
    parent->addChild(layoutView);
  }

  for (auto& [key, val] : j.items())
  {
    auto path = currentPath;
    path /= key;

    createOneOrMoreLayoutViews(val, path, layoutView);
  }

  return layoutView;
}

void Layout::Layout::createLayoutViews(const nlohmann::json& j,
                                       json::json_pointer currentPath,
                                       std::shared_ptr<LayoutView> parent)
{
  if (!j.is_array())
  {
    WARN("create layout views from json array, json is not array, return");
    return;
  }

  auto size = j.size();
  for (auto i = 0; i < size; ++i)
  {
    auto path = currentPath;
    path /= i;

    createOneOrMoreLayoutViews(j[i], path, parent);
  }
}

void Layout::Layout::createOneOrMoreLayoutViews(const nlohmann::json& j,
                                                json::json_pointer currentPath,
                                                std::shared_ptr<LayoutView> parent)
{
  if (j.is_object())
  {
    createOneLayoutView(j, currentPath, parent);
  }
  else if (j.is_array())
  {
    createLayoutViews(j, currentPath, parent);
  }
}

void Layout::Layout::collectRules(const nlohmann::json& json)
{
  if (!json.is_object())
  {
    return;
  }

  auto& obj = json["obj"];
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

void Layout::Layout::configureAutoLayout(std::shared_ptr<LayoutView> view)
{
  std::shared_ptr<VGG::Layout::Internal::Rule::Rule> rule;

  auto json = m_designJson[nlohmann::json::json_pointer{ view->path() }];
  if (json.contains(K_ID))
  {
    auto nodeId = json.at(K_ID).get<std::string>();
    if (m_rules.find(nodeId) != m_rules.end())
    {
      rule = m_rules[nodeId];
    }
  }

  auto autoLayout = view->createAutoLayout();
  if (rule)
  {
    autoLayout->rule = rule;
    view->configureAutoLayout();
  }

  for (auto& child : view->children())
  {
    configureAutoLayout(child);
  }
}
