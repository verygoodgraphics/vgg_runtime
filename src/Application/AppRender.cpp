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
#include "Application/AppRender.hpp"
#include "AppRenderable.hpp"
#include "AppScene.hpp"
#include "Event/EventListener.hpp"
#include "Utility/CappingProfiler.hpp"

#include "Layer/VGGLayer.hpp"

namespace VGG::app
{

class AppRender__pImpl
{
public:
  VGG_DECL_API(AppRender)
  std::vector<std::shared_ptr<AppScene>> listeners;
  std::vector<std::shared_ptr<AppRenderable>> renderables;
  std::queue<std::pair<UEvent, void*>> msgQueue;
  AppRender__pImpl(AppRender* api)
    : q_ptr(api)
  {
  }

  void dispatchEvent(UEvent e, void* userData)
  {
    q_ptr->onEvent(e);
    for (const auto& l : listeners)
    {
      l->onEvent(e, userData);
    }
    for (const auto& l : renderables)
    {
      l->onEvent(e, userData);
    }
  }

  ~AppRender__pImpl()
  {
  }
};

AppRender::AppRender()
  : d_ptr(new AppRender__pImpl(this))
{
}

void AppRender::postEvent(UEvent e, void* userData)
{
  VGG_IMPL(AppRender)
  _->msgQueue.emplace(e, userData);
}
void AppRender::sendEvent(UEvent e, void* userData)
{
  VGG_IMPL(AppRender)
  _->dispatchEvent(e, userData);
}
void AppRender::addAppScene(std::shared_ptr<AppScene> listener)
{
  VGG_IMPL(AppRender)
  _->listeners.push_back(listener);
  addScene(std::move(listener));
}

void AppRender::addAppRenderable(std::shared_ptr<AppRenderable> listener)
{
  VGG_IMPL(AppRender)
  _->renderables.push_back(listener);
  addRenderItem(std::move(listener));
}

bool AppRender::beginFrame(int fps)
{
  VGG_IMPL(AppRender);
  while (_->msgQueue.empty() == false)
  {
    auto& [e, u] = _->msgQueue.front();
    _->dispatchEvent(e, u);
    _->msgQueue.pop();
  }

  auto profiler = CappingProfiler::getInstance();
  if (fps > 0)
  {
    if (!(profiler->enoughFrameDuration(fps)))
    {
      return false;
    }
  }
  profiler->markFrame();
  VLayer::beginFrame();
  return true;
}
void AppRender::render()
{
  VLayer::render();
}
void AppRender::endFrame()
{
  VLayer::endFrame();
}

bool AppRender::onEvent(UEvent e)
{
  auto type = e.type;
  if (type == VGG_MOUSEMOTION)
  {
    if (enableDrawPosition())
    {
      drawPosition(e.motion.windowX, e.motion.windowY);
    }
  }
  if (type == EEventType::VGG_APP_INIT)
  {
    // for GUI front-end, an init event is necessary to trigger resize to create surface.
    DEBUG("Layer Initiailization: [%d, %d]", e.init.drawableWidth, e.init.drawableHeight);
    resize(e.init.drawableWidth, e.init.drawableHeight);
  }
  return false;
}

AppRender::~AppRender()
{
  shutdown();
}

} // namespace VGG::app
