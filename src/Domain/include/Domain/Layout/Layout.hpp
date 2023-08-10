#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/Layout/View.hpp"
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
  nlohmann::json m_design_json;

public:
  Layout(std::shared_ptr<Daruma> model)
    : m_model{ model }
  {
    ASSERT(m_model);

    m_design_json = m_model->designDoc()->content();
  }

  nlohmann::json designDoc()
  {
    return m_design_json;
  }

  void layout(Size size);
  std::shared_ptr<LayoutView> createLayoutTree();

private:
  std::shared_ptr<LayoutView> createOneLayoutView(const nlohmann::json& j,
                                                  nlohmann::json::json_pointer current_path,
                                                  std::shared_ptr<LayoutView> parent);
  void createLayoutViews(const nlohmann::json& j,
                         nlohmann::json::json_pointer current_path,
                         std::shared_ptr<LayoutView> parent);

  void createOneOrMoreLayoutViews(const nlohmann::json& j,
                                  nlohmann::json::json_pointer current_path,
                                  std::shared_ptr<LayoutView> parent);
};

} // namespace Layout
} // namespace VGG
