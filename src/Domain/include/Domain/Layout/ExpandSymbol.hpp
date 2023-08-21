#pragma once

#include "Rect.hpp"

#include "nlohmann/json.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace VGG
{
namespace Layout
{
constexpr auto k_separator = "__";

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
  void expand_instance(nlohmann::json& json,
                       std::vector<std::string>& instance_id_stack,
                       bool again = false);
  void scale_from_master(nlohmann::json& instance, nlohmann::json& master);

  void normalize_children_geometry(nlohmann::json& json, const Size container_size);
  void recalculate_intance_children_geometry(nlohmann::json& json, Size container_size);

  void apply_overrides(nlohmann::json& instance, const std::vector<std::string>& instance_id_stack);
  void apply_overrides(nlohmann::json& json,
                       std::stack<std::string> reversed_path,
                       const nlohmann::json& value);
  nlohmann::json* find_child_object(nlohmann::json& json, const std::string& object_id);
  void make_id_unique(nlohmann::json& json, const std::string& id_prefix);
  void make_mask_id_unique(nlohmann::json& json,
                           nlohmann::json& instance_json,
                           const std::string& id_prefix);
  std::string join(const std::vector<std::string>& instance_id_stack,
                   const std::string& seperator = k_separator);
};
} // namespace Layout

} // namespace VGG