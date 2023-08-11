#include "ExpandSymbol.hpp"

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
  auto size_is_equal = true;
  if (size_is_equal)
  {
    return;
  }

  normalize_children_geometry_within_master(instance, master);
  recalculate_intance_children_geometry(instance, master);
}

void ExpandSymbol::normalize_children_geometry_within_master(nlohmann::json& instance,
                                                             nlohmann::json& master)
{
}

void ExpandSymbol::recalculate_intance_children_geometry(nlohmann::json& instance,
                                                         nlohmann::json& master)
{
}

void ExpandSymbol::apply_overrides(nlohmann::json& instance, nlohmann::json& master)
{
  override_master(instance);

  // other override
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
        (*child_instance)[k_override_values] = nl::json(nl::detail::value_t::array);
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