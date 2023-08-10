#pragma once

#include "nlohmann/json.hpp"

namespace VGG
{

namespace Layout
{
class ExpandSymbol
{
  nlohmann::json m_design_json;

public:
  ExpandSymbol(const nlohmann::json& design_json)
    : m_design_json{ design_json }
  {
  }

  nlohmann::json operator()();
};
} // namespace Layout

} // namespace VGG