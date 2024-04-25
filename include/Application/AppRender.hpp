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
#include "Event/Event.hpp"
#include "AppScene.hpp"
#include "AppRenderable.hpp"

#include "Layer/VGGLayer.hpp"

#include <queue>

namespace VGG::app
{

class AppRender__pImpl;
class AppRender : public layer::VLayer
{
  VGG_DECL_IMPL(AppRender)

public:
  AppRender();
  void postEvent(UEvent e, void* userData);
  void sendEvent(UEvent e, void* userData);
  void addAppRenderable(std::shared_ptr<AppRenderable> listener);
  void addEventListener(std::shared_ptr<EventListener> listener);

  std::shared_ptr<AppScene> popAppScene();
  using VLayer::beginFrame;
  bool beginFrame(int fps);
  void render();
  void endFrame();

  ~AppRender();

protected:
  bool onEvent(UEvent e);
};

} // namespace VGG::app
