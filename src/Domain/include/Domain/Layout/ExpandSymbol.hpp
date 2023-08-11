#pragma once

#include "nlohmann/json.hpp"

#include <string>
#include <unordered_map>

namespace VGG
{
namespace Layout
{
constexpr auto k_child_objects = "childObjects";
constexpr auto k_class = "class";
constexpr auto k_empty_string = "";
constexpr auto k_id = "id";
constexpr auto k_master_id = "masterId";
constexpr auto k_object_id = "objectId";
constexpr auto k_override_class = "overrideValue";
constexpr auto k_override_name = "overrideName";
constexpr auto k_override_value = "overrideValue";
constexpr auto k_override_values = "overrideValues";
constexpr auto k_symbol_instance = "symbolInstance";
constexpr auto k_symbol_master = "symbolMaster";

class ExpandSymbol
{
  const nlohmann::json m_design_json;
  std::unordered_map<std::string, nlohmann::json> m_masters;

public:
  ExpandSymbol(const nlohmann::json& design_json)
    : m_design_json{ design_json }
  {
  }

  nlohmann::json operator()();

private:
  void collect_master(const nlohmann::json& json);
  void expand_instance(nlohmann::json& json);
  void scale_from_master(nlohmann::json& instance, nlohmann::json& master);
  void normalize_children_geometry_within_master(nlohmann::json& instance, nlohmann::json& master);
  void recalculate_intance_children_geometry(nlohmann::json& instance, nlohmann::json& master);
  void apply_overrides(nlohmann::json& instance, nlohmann::json& master);
  void override_master(nlohmann::json& instance);
  nlohmann::json* find_child_instance(nlohmann::json& json, const nlohmann::json& object_id);
};
} // namespace Layout

} // namespace VGG