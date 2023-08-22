#include "Layout.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"

using namespace VGG;

constexpr auto FLIP_Y_FACTOR = -1;

void Layout::Layout::layout(Size size)
{
  if (size != m_size)
  {
    // todo
    m_size = size;
  }
}

std::shared_ptr<LayoutView> Layout::Layout::layoutTree()
{
  if (!m_designJson.is_object())
  {
    WARN("invalid design file");
    return {};
  }

  m_layout_tree.reset(new LayoutView{ "/", {} });

  json::json_pointer frames_path{ "/frames" };
  for (auto i = 0; i < m_design_json[K_FRAMES].size(); ++i)
  {
    auto path = frames_path / i;
    createOneLayoutView(m_design_json[path], path, m_layout_tree);
  }

  return m_layout_tree;
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
      auto& id = item[k_id];
      m_rules[id] = item;
    }
  }
}