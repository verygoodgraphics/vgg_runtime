#pragma once

#include "Rect.hpp"

#include "nlohmann/json.hpp"

#include <string>
#include <unordered_map>

namespace VGG
{
namespace Layout
{

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

  void normalize_children_geometry(nlohmann::json& json, const Size container_size);
  void recalculate_intance_children_geometry(nlohmann::json& json, Size container_size);

  void apply_overrides(nlohmann::json& instance, nlohmann::json& master);
  nlohmann::json* find_child_object(nlohmann::json& json, const nlohmann::json& object_id);

  Rect get_node_bounds(nlohmann::json& node_json);
};
} // namespace Layout

} // namespace VGG