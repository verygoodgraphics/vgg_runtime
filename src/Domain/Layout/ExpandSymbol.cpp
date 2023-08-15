#include "ExpandSymbol.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"

#include <iostream>

namespace nl = nlohmann;
using jref = nl::detail::json_ref<nl::json>;
using namespace VGG::Layout;

nlohmann::json ExpandSymbol::operator()()
{
  collect_master(m_design_json);

  auto result = m_design_json;
  expand_instance(result);

  return result;
}

void ExpandSymbol::collect_master(const nlohmann::json& json)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (json.is_object())
  {
    auto class_name = json.value(k_class, k_empty_string);
    if (class_name == k_symbol_master)
    {
      auto id = json[k_id];
      m_masters[id] = json;
    }
  }

  for (auto& el : json.items())
  {
    collect_master(el.value());
  }
}

void ExpandSymbol::expand_instance(nlohmann::json& json)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (json.is_object())
  {
    auto class_name = json.value(k_class, k_empty_string);
    if (class_name == k_symbol_instance)
    {
      auto master_id = json[k_master_id].get<std::string>();
      if (m_masters.find(master_id) != m_masters.end())
      {
        auto& master_json = m_masters[master_id];
        json[k_child_objects] = master_json[k_child_objects];

        expand_instance(json[k_child_objects]);

        apply_overrides(json, master_json);

        scale_from_master(json, master_json);

        // make instance node to "symbalMaster" or render will not draw this node
        json[k_class] = k_symbol_master;
        json.erase(k_master_id);
        json.erase(k_override_values);
      }
    }
  }

  for (auto& el : json.items())
  {
    expand_instance(el.value());
  }
}

void ExpandSymbol::scale_from_master(nlohmann::json& instance, nlohmann::json& master)
{
  auto master_size = master[k_bounds].get<Rect>().size;
  auto instance_size = instance[k_bounds].get<Rect>().size;

  auto size_is_equal = false;
  if (master_size == instance_size)
  {
    return;
  }

  normalize_children_geometry(instance[k_child_objects], master_size);
  recalculate_intance_children_geometry(instance[k_child_objects], instance_size);
}

void ExpandSymbol::normalize_children_geometry(nlohmann::json& json, const Size container_size)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }
  auto my_container_size = container_size;
  auto child_container_size = container_size;

  auto has_bounds = is_layout_node(json);
  Rect bounds, frame;
  Matrix matrix;
  if (has_bounds)
  {
    bounds = json[k_bounds].get<Rect>();
    frame = json[k_frame].get<Rect>();
    matrix = json[k_matrix].get<Matrix>();

    child_container_size = bounds.size;
  }

  // bottom up; leaf first
  for (auto& el : json.items())
  {
    normalize_children_geometry(el.value(), child_container_size);
  }

  // root last, normalize self
  if (has_bounds)
  {
    Rect normalized_bounds = {
      { bounds.origin.x / my_container_size.width, bounds.origin.y / my_container_size.height },
      { bounds.size.width / my_container_size.width, bounds.size.height / my_container_size.height }
    };
    Rect normalized_frame = {
      { frame.origin.x / my_container_size.width, frame.origin.y / my_container_size.height },
      { frame.size.width / my_container_size.width, frame.size.height / my_container_size.height }
    };
    Matrix normalized_matrix = matrix;
    normalized_matrix.tx = matrix.tx / my_container_size.width;
    normalized_matrix.ty = matrix.ty / my_container_size.height;

    to_json(json[k_bounds], normalized_bounds);
    to_json(json[k_frame], normalized_frame);
    to_json(json[k_matrix], normalized_matrix);
  }
}

void ExpandSymbol::recalculate_intance_children_geometry(nlohmann::json& json, Size container_size)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  // top down, root fist
  auto has_bounds = is_layout_node(json);
  Rect normalized_bounds, normalized_frame;
  Matrix normalized_matrix;
  if (has_bounds)
  {
    normalized_bounds = json[k_bounds].get<Rect>();
    normalized_frame = json[k_frame].get<Rect>();
    normalized_matrix = json[k_matrix].get<Matrix>();

    Rect bounds = { { normalized_bounds.origin.x * container_size.width,
                      normalized_bounds.origin.y * container_size.height },
                    { normalized_bounds.size.width * container_size.width,
                      normalized_bounds.size.height * container_size.height } };
    Rect frame = { { normalized_frame.origin.x * container_size.width,
                     normalized_frame.origin.y * container_size.height },
                   { normalized_frame.size.width * container_size.width,
                     normalized_frame.size.height * container_size.height } };
    auto matrix = normalized_matrix;
    matrix.tx = normalized_matrix.tx * container_size.width;
    matrix.ty = normalized_matrix.ty * container_size.height;

    to_json(json[k_bounds], bounds);
    to_json(json[k_frame], frame);
    to_json(json[k_matrix], matrix);

    container_size = bounds.size;
  }

  // leaf last
  for (auto& el : json.items())
  {
    recalculate_intance_children_geometry(el.value(), container_size);
  }
}

void ExpandSymbol::apply_overrides(nlohmann::json& instance, nlohmann::json& master)
{
  override_master(instance);

  // todo: other override
}

void ExpandSymbol::override_master(nlohmann::json& instance)
{
  auto& override_values = instance[k_override_values];
  if (!override_values.is_array())
  {
    return;
  }

  for (auto& el : override_values.items())
  {
    auto& override_item = el.value();
    if (override_item.is_object() && (override_item[k_class] == k_override_class) &&
        (override_item[k_override_name] == k_master_id))
    {
      nl::json* child_instance =
        find_child_instance(instance[k_child_objects], override_item[k_object_id]);
      if (child_instance && child_instance->is_object())
      {
        (*child_instance)[k_master_id] = override_item[k_override_value];
        (*child_instance).erase(k_override_values);
        expand_instance(*child_instance);
      }
    }
  }
}

nlohmann::json* ExpandSymbol::find_child_instance(nlohmann::json& json,
                                                  const nlohmann::json& object_id)
{
  if (!json.is_object() && !json.is_array())
  {
    return nullptr;
  }

  if (json.is_object())
  {
    auto class_name = json.value(k_class, k_empty_string);
    if ((class_name == k_symbol_instance) && (json[k_id] == object_id))
    {
      return &json;
    }
  }

  for (auto& el : json.items())
  {
    auto ret = find_child_instance(el.value(), object_id);
    if (ret)
    {
      return ret;
    }
  }

  return nullptr;
}