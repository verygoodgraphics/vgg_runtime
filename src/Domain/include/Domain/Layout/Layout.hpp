#pragma once

#include "Domain/Daruma.hpp"
#include "ExpandSymbol.hpp"
#include "Log.h"
#include "View.hpp"

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
  nlohmann::json m_designJson;
  Size m_size;
  std::shared_ptr<LayoutView> m_layoutTree;
  std::unordered_map<std::string, std::shared_ptr<Internal::Rule::Rule>> m_rules;

public:
  Layout(std::shared_ptr<Daruma> model)
    : m_model{ model }
  {
    ASSERT(m_model);

    m_designJson = ExpandSymbol{ m_model->designDoc()->content() }();
    if (m_model->layoutDoc())
    {
      collectRules(m_model->layoutDoc()->content());
    }
  }

  const nlohmann::json& designDoc()
  {
    return m_designJson;
  }

  void layout(Size size);
  std::shared_ptr<LayoutView> layoutTree();

private:
  std::shared_ptr<LayoutView> createOneLayoutView(const nlohmann::json& j,
                                                  nlohmann::json::json_pointer currentPath,
                                                  std::shared_ptr<LayoutView> parent);
  void createLayoutViews(const nlohmann::json& j,
                         nlohmann::json::json_pointer currentPath,
                         std::shared_ptr<LayoutView> parent);

  void createOneOrMoreLayoutViews(const nlohmann::json& j,
                                  nlohmann::json::json_pointer currentPath,
                                  std::shared_ptr<LayoutView> parent);
  void collectRules(const nlohmann::json& json);
  void configureView(std::shared_ptr<LayoutView> view);
};

} // namespace Layout
} // namespace VGG
