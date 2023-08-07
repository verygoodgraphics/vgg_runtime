#include "Layout.hpp"

#include "Log.h"

using namespace VGG;

constexpr auto flip_y_factor = -1;

void Layout::Layout::layout(Size size)
{
}

nlohmann::json Layout::Layout::normalizePoint()
{
  return m_model->designDoc()->content();
}

std::shared_ptr<LayoutView> Layout::Layout::createLayoutTree()
{
  auto doc = m_model->designDoc()->content();

  // todo, select artboard
  auto& artboard = doc["artboard"][0];
  if (!artboard.is_object())
  {
    WARN("no artboard in design file");
    return {};
  }

  return createOneLayoutView(artboard, json::json_pointer("/artboard/0"), nullptr);
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

  // check top level class
  auto class_name = j.value("class", "");
  if (class_name != "artboard" && class_name != "frame" && class_name != "group" &&
      class_name != "image" && class_name != "layer" && class_name != "path" &&
      class_name != "symbolInstance" && class_name != "symbolMaster" && class_name != "text")
  {
    return nullptr;
  }

  Rect frame;
  constexpr auto frame_key = "frame";
  const auto it = j.find(frame_key);
  if (it != j.end())
  {
    auto frame_json = j[frame_key];
    auto x = frame_json["x"];
    auto y = frame_json["y"].get<Scalar>() * flip_y_factor;
    auto width = frame_json["width"];
    auto height = frame_json["height"];

    frame = { { x, y }, { width, height } };
  }

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