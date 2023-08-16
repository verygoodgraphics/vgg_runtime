#include "ExpandSymbol.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"

#include <iostream>
#include <stack>

#undef DEBUG
#define DEBUG(msg, ...)

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
        DEBUG("#ExpandSymbol: expand instance[id=%s, ptr=%p] with masterId=%s",
              json[k_id].get<std::string>().c_str(),
              &json,
              master_id.c_str());
        auto& master_json = m_masters[master_id];
        json[k_child_objects] = master_json[k_child_objects];

        expand_instance(json[k_child_objects]);

        apply_overrides(json);

        scale_from_master(json, master_json);

        // make instance node to "symbalMaster" or render will not draw this node
        DEBUG("#ExpandSymbol: make instance[id=%s, ptr=%p] as master, erase masterId",
              json[k_id].get<std::string>().c_str(),
              &json);
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
  if (is_point_attr_node(json))
  {
    auto point = json[k_point].get<Point>();
    point.x /= my_container_size.width;
    point.y /= my_container_size.height;
    json[k_point] = point;
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
  if (is_point_attr_node(json))
  {
    auto point = json[k_point].get<Point>();
    point.x *= container_size.width;
    point.y *= container_size.height;
    json[k_point] = point;
  }

  // leaf last
  for (auto& el : json.items())
  {
    recalculate_intance_children_geometry(el.value(), container_size);
  }
}

void ExpandSymbol::apply_overrides(nlohmann::json& instance)
{
  auto& override_values = instance[k_override_values];
  if (!override_values.is_array())
  {
    return;
  }

  for (auto& el : override_values.items())
  {
    auto& override_item = el.value();
    if (!override_item.is_object() || (override_item[k_class] != k_override_class))
    {
      continue;
    }

    nl::json* child_object =
      find_child_object(instance[k_child_objects], override_item[k_object_id]);
    if (!child_object || !child_object->is_object())
    {
      continue;
    }
    std::string name = override_item[k_override_name];
    auto value = override_item[k_override_value];
    if (name == k_master_id) // override masterId
    {
      DEBUG(
        "#ExpandSymbol: overide instance[id=%s, ptr=%p], old masterId=%s, new masterId=%s, restore "
        "class to symbolInstance to expand",
        (*child_object)[k_id].dump().c_str(),
        child_object,
        (*child_object)[k_master_id].dump().c_str(),
        value.get<std::string>().c_str());
      (*child_object)[k_master_id] = value;
      (*child_object).erase(k_override_values);

      // restore to symbolInstance to expand again
      (*child_object)[k_class] = k_symbol_instance;
      expand_instance(*child_object);
    }
    else if (!name.empty()) // other overrides
    {
      // make name to json pointer string
      while (true)
      {
        auto index = name.find(".");
        if (index == std::string::npos)
        {
          break;
        }
        name[index] = '/';
      }

      nl::json::json_pointer path{ "/" + name };
      DEBUG("#ExpandSymbol: override object[id=%s, ptr=%p], path=%s, value=%s",
            (*child_object)[k_id].get<std::string>().c_str(),
            child_object,
            path.to_string().c_str(),
            value.dump().c_str());
      if (name.find("*") == std::string::npos) // no * in path
      {
        (*child_object)[path] = value;
      }
      else // path has *
      {
        std::stack<std::string> path_stack;
        while (!path.empty())
        {
          path_stack.push(path.back());
          path.pop_back();
        }

        apply_overrides((*child_object), path_stack, value);
      }
    }
  }
}

void ExpandSymbol::apply_overrides(nlohmann::json& json,
                                   std::stack<std::string> reversed_path,
                                   const nlohmann::json& value)
{
  ASSERT(!reversed_path.empty());

  auto key = reversed_path.top();
  reversed_path.pop();
  auto is_last_key = reversed_path.empty();

  if (key != "*")
  {
    if (is_last_key)
    {
      json[key] = value;
    }
    else
    {
      apply_overrides(json[key], reversed_path, value);
    }
  }
  else
  {
    for (auto& el : json.items())
    {
      if (is_last_key)
      {
        el.value() = value;
      }
      else
      {
        apply_overrides(el.value(), reversed_path, value);
      }
    }
  }
}

nlohmann::json* ExpandSymbol::find_child_object(nlohmann::json& json,
                                                const nlohmann::json& object_id)
{
  if (!json.is_object() && !json.is_array())
  {
    return nullptr;
  }

  if (json.is_object())
  {
    if (json[k_id] == object_id)
    {
      return &json;
    }
  }

  for (auto& el : json.items())
  {
    auto ret = find_child_object(el.value(), object_id);
    if (ret)
    {
      return ret;
    }
  }

  return nullptr;
}