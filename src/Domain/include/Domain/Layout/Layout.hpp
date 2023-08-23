#pragma once

#include "Domain/Daruma.hpp"
#include "ExpandSymbol.hpp"
#include "View.hpp"
#include "Log.h"

#include "nlohmann/json.hpp"

#include <memory>

namespace VGG
{
namespace Layout
{

class Layout
{
  std::shared_ptr<Daruma> m_model;
  nlohmann::json m_designJson;

public:
  Layout(std::shared_ptr<Daruma> model)
    : m_model{ model }
  {
    ASSERT(m_model);

    m_designJson = m_model->designDoc()->content();
  }

  nlohmann::json designDoc()
  {
    ExpandSymbol expandeSymbol{ m_designJson };
    return expandeSymbol();
  }

  void layout(Size size);
  std::shared_ptr<LayoutView> createLayoutTree();

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
};

} // namespace Layout
} // namespace VGG
