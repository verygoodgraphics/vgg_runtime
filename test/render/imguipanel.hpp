#pragma once
#include "Layer/Core/SceneNode.hpp"
#include "imgui_integration.hpp"

#include "Layer/Core/RenderNode.hpp"
#include "Layer/Core/ViewportNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "viewer.hpp"

namespace vl = VGG::layer;
class Panel : public ImGuiPanel
{
  Viewer&        m_viewer;
  vl::SceneNode* m_scene{ nullptr };
  bool           m_show = true;

public:
  Panel(Viewer& viewer)
    : ImGuiPanel()
    , m_viewer(viewer)
  {
  }

  void setScene(vl::SceneNode* scene)
  {
    m_scene = scene;
  }

  void setVisible(bool visible)
  {
    m_show = visible;
  }

  bool isVisible() const
  {
    return m_show;
  }

  void draw() override
  {
    if (!m_show)
      return;
    ImGui::Begin("Panel");
    ImGui::Text("Frame count: %d", (int)m_scene->getFrames().size());
    ImGui::End();
  }
};
