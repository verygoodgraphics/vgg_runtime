#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/Layout/View.hpp"

#include "nlohmann/json.hpp"

#include <memory>

namespace VGG
{
namespace Layout
{

class Layout
{
  std::shared_ptr<Daruma> m_model;

public:
  Layout(std::shared_ptr<Daruma> model)
    : m_model{ model }
  {
  }

  void layout(Size size);
  nlohmann::json normalizePoint();
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

  void normalizePoint(nlohmann::json& json);
};

} // namespace Layout
} // namespace VGG
