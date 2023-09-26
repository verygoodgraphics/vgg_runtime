#pragma once

#include "Domain/Daruma.hpp"
#include "ExpandSymbol.hpp"
#include "Utility/Log.hpp"
#include "Node.hpp"

#include <nlohmann/json.hpp>

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
  JsonDocumentPtr m_designDoc;
  JsonDocumentPtr m_layoutDoc;
  std::shared_ptr<LayoutNode> m_layoutTree;
  std::unordered_map<std::string, std::shared_ptr<Internal::Rule::Rule>> m_rules;
  bool m_isRootTree{ true }; // root document or fragment
  std::vector<Size> m_pageSize;

public:
  Layout(JsonDocumentPtr designDoc, JsonDocumentPtr layoutDoc, bool isRootTree = true);

  void layout(Size size);
  std::shared_ptr<LayoutNode> layoutTree()
  {
    return m_layoutTree;
  }

  auto pageSize(int index)
  {
    return m_pageSize[index];
  }

private:
  void buildLayoutTree();
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
