#include "Layout.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"

using namespace VGG;

constexpr auto flip_y_factor = -1;

void Layout::Layout::layout(Size size)
{
  // todo
}

std::shared_ptr<LayoutView> Layout::Layout::createLayoutTree()
{
  auto doc = m_model->designDoc()->content();

  // todo, select frame
  auto& frame = doc[k_frame][0];
  if (!frame.is_object())
  {
    WARN("no frame in design file");
    return {};
  }

  return createOneLayoutView(frame, json::json_pointer("/frames/0"), nullptr);
}

std::shared_ptr<LayoutView> Layout::Layout::createOneLayoutView(const nlohmann::json& j,
                                                                json::json_pointer current_path,
                                                                std::shared_ptr<LayoutView> parent)
{
  if (!j.is_object())
  {
    WARN("create one layout view from json object, json is not object, return");
    return nullptr;
  }

  if (!is_layout_node(j))
  {
    return nullptr;
  }

  auto frame = j[k_frame].get<Rect>();
  frame.origin.y *= flip_y_factor;

  auto layout_view = std::make_shared<LayoutView>(current_path.to_string(), frame);
  if (parent)
  {
    parent->addChild(layout_view);
  }

  for (auto& [key, val] : j.items())
  {
    auto path = current_path;
    path /= key;

    createOneOrMoreLayoutViews(val, path, layout_view);
  }

  return layout_view;
}

void Layout::Layout::createLayoutViews(const nlohmann::json& j,
                                       json::json_pointer current_path,
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
    auto path = current_path;
    path /= i;

    createOneOrMoreLayoutViews(j[i], path, parent);
  }
}

void Layout::Layout::createOneOrMoreLayoutViews(const nlohmann::json& j,
                                                json::json_pointer current_path,
                                                std::shared_ptr<LayoutView> parent)
{
  if (j.is_object())
  {
    createOneLayoutView(j, current_path, parent);
  }
  else if (j.is_array())
  {
    createLayoutViews(j, current_path, parent);
  }
}
