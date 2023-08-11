#include "ExpandSymbol.hpp"

#include <iostream>

namespace nl = nlohmann;
using jref = nl::detail::json_ref<nl::json>;
using namespace VGG::Layout;

constexpr auto k_child_objects = "childObjects";
constexpr auto k_class = "class";
constexpr auto k_empty_string = "";
constexpr auto k_id = "id";
constexpr auto k_master_id = "masterId";
constexpr auto k_symbol_instance = "symbolInstance";
constexpr auto k_symbol_master = "symbolMaster";

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
        scale_from_master(json, master_json);
        handle_override(json, master_json);
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

void ExpandSymbol::handle_override(nlohmann::json& instance, nlohmann::json& master)
{
}

void ExpandSymbol::handle_override_master_id(nlohmann::json& instance, nlohmann::json& master)
{
}