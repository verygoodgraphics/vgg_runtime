/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
class UIScrollView;
}

class UIApplication : public app::EventListener
{
  app::AppRender*               m_layer{ nullptr };
  std::shared_ptr<UIScrollView> m_view;
  std::shared_ptr<Controller>   m_controller;

  bool m_firstRender{ true };

public:
  void setLayer(app::AppRender* layer);
  void setView(std::shared_ptr<UIScrollView> view, double w, double h);

  void setController(std::shared_ptr<Controller> controller)
  {
    ASSERT(controller);
    m_controller = controller;

    ASSERT(m_layer);
    m_layer->addAppRenderable(m_controller->editor());
  }

  bool onEvent(UEvent evt, void* userData) override;

  bool needsPaint();
  bool paint(int fps, bool force = false);

private:
  bool handleKeyEvent(VKeyboardEvent evt);
};
