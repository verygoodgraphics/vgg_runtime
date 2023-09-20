#pragma once

#include "Editor.hpp"

#include <nlohmann/json.hpp>

#include <memory>

class VggExec;

namespace VGG
{

class Reporter : public Editor::Listener
{
  std::weak_ptr<VggExec> m_jsEngine;

public:
  Reporter(std::weak_ptr<VggExec> jsEngine)
    : m_jsEngine{ jsEngine }
  {
  }

  void onSelectNode(std::weak_ptr<LayoutNode> node) override;

private:
  void sendEventToJs(const nlohmann::json& event);
};

} // namespace VGG