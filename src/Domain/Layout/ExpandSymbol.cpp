#include "ExpandSymbol.hpp"

using namespace VGG::Layout;

nlohmann::json ExpandSymbol::operator()()
{
  return m_design_json;
}