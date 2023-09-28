#pragma once

#include "Application/AppRender.hpp"
#include "Application/Controller.hpp"
#include "Utility/ConfigManager.hpp"
#include "Event/Event.hpp"
#include "Event/EventListener.hpp"
#include "Event/Keycode.hpp"

#include "Layer/Scene.hpp"
#include "Layer/VGGLayer.hpp"

#include <nlohmann/json.hpp>

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>

namespace fs = std::filesystem;
using namespace VGG;

namespace VGG
{
class UIView;
}

class UIApplication : public app::EventListener
{
  app::AppRender* m_layer{ nullptr };
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<Controller> m_controller;

  bool m_firstRender{ true };

public:
  void setLayer(app::AppRender* layer)
  {
    ASSERT(layer);
    m_layer = layer;
  }

  void setView(std::shared_ptr<UIView> view);

  void setController(std::shared_ptr<Controller> controller)
  {
    ASSERT(controller);
    m_controller = controller;

    ASSERT(m_layer);
    m_layer->addAppRenderable(m_controller->editor());
  }

  bool onEvent(UEvent evt, void* userData) override;

  void run(int fps);
};
