/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "AppZoomer.hpp"
#include "Event/EventListener.hpp"

#include "Layer/Scene.hpp"

namespace VGG::app
{

class AppScene
  : public Scene
  , public EventListener
{
protected:
  std::shared_ptr<AppZoomer> m_zoomerListener;

public:
  AppScene(std::unique_ptr<Rasterizer> cache = nullptr)
    : Scene(std::move(cache))
  {
  }
  // By setting a zoomer listener to customize your own operation
  void setZoomerListener(std::shared_ptr<AppZoomer> zoomerListener)
  {
    m_zoomerListener = std::move(zoomerListener);
    setZoomer(m_zoomerListener);
  }

  bool onEvent(UEvent e, void* userData) override
  {
    if (m_zoomerListener)
      return m_zoomerListener->onEvent(e, userData);
    return false;
  }
};
} // namespace VGG::app
