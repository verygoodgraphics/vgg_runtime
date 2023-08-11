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
};
} // namespace Layout

} // namespace VGG