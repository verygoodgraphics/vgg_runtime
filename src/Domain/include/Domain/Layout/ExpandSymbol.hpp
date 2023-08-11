#pragma once

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
  void normalize_children_geometry_within_master(nlohmann::json& instance, nlohmann::json& master);
  void recalculate_intance_children_geometry(nlohmann::json& instance, nlohmann::json& master);
  void handle_override(nlohmann::json& instance, nlohmann::json& master);
  void handle_override_master_id(nlohmann::json& instance, nlohmann::json& master);
};
} // namespace Layout

} // namespace VGG