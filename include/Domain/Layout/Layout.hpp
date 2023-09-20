#pragma once

#include "Domain/Daruma.hpp"
#include "ExpandSymbol.hpp"
#include "Utility/Log.h"
#include "Node.hpp"

#include "nlohmann/json.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace VGG
{
namespace Layout
{
namespace Internal
{
namespace Rule
{
struct Rule;
}
} // namespace Internal

class Layout
{
  std::shared_ptr<Daruma> m_model;
  Size m_size;
  std::shared_ptr<LayoutNode> m_layoutTree;
  std::unordered_map<std::string, std::shared_ptr<Internal::Rule::Rule>> m_rules;

public:
  Layout(std::shared_ptr<Daruma> model);

  void layout(Size size);
  std::shared_ptr<LayoutNode> layoutTree();

private:
  std::shared_ptr<LayoutNode> createOneLayoutNode(const nlohmann::json& j,
                                                  nlohmann::json::json_pointer currentPath,
                                                  std::shared_ptr<LayoutNode> parent);
  void createLayoutNodes(const nlohmann::json& j,
                         nlohmann::json::json_pointer currentPath,
                         std::shared_ptr<LayoutNode> parent);

  void createOneOrMoreLayoutNodes(const nlohmann::json& j,
                                  nlohmann::json::json_pointer currentPath,
                                  std::shared_ptr<LayoutNode> parent);
  void collectRules(const nlohmann::json& json);
  void configureNodeAutoLayout(std::shared_ptr<LayoutNode> node);
};

} // namespace Layout
} // namespace VGG
