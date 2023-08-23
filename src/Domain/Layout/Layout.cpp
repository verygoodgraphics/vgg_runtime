#include "Layout.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"

using namespace VGG;

constexpr auto FLIP_Y_FACTOR = -1;

void Layout::Layout::layout(Size size)
{
  // todo
}

std::shared_ptr<LayoutView> Layout::Layout::createLayoutTree()
{
  auto doc = m_model->designDoc()->content();

  // todo, select frame
  auto& frame = doc[K_FRAME][0];
  if (!frame.is_object())
  {
    WARN("no frame in design file");
    return {};
  }

  return createOneLayoutView(frame, json::json_pointer("/frames/0"), nullptr);
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
