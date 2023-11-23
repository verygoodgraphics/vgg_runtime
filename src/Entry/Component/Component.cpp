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

#include "../Common/FakeMouse.hpp"
#include "Component.hpp"
#include "ComponentComposer.hpp"

#include "Application/AppRender.hpp"
#include "Application/MainComposer.hpp"
#include "Application/RunLoop.hpp"
#include "Application/UIApplication.hpp"
#include "Application/UIView.hpp"
#include "Layer/Graphics/ContextSkBase.hpp"

namespace VGG
{

// impl ----------------------------------------------------------------------
class ComponentImpl
{
  Component* m_api;

  std::shared_ptr<app::AppRender>             m_appRender;
  std::unique_ptr<layer::SkiaGraphicsContext> m_context;

  std::unique_ptr<MainComposer>  m_mainComposer;
  std::unique_ptr<UIApplication> m_application;

  layer::ContextConfig m_graphicsContextConfig;

public:
  ComponentImpl(Component* api)
    : m_api(api)
  {
    m_mainComposer.reset(
      new MainComposer{ new ComponentComposer{}, std::make_shared<FakeMouse>() });
    m_application.reset(new UIApplication);

    // todo, event listener

    // todo, init layer
    m_appRender = std::make_shared<app::AppRender>();
    m_application->setLayer(m_appRender.get());

    m_application->setView(m_mainComposer->view());
    m_application->setController(m_mainComposer->controller());
  }

  bool load(const std::string& filePath,
            const char*        designDocSchemaFilePath = nullptr,
            const char*        layoutDocSchemaFilePath = nullptr)
  {
    return m_mainComposer->controller()->start(filePath,
                                               designDocSchemaFilePath,
                                               layoutDocSchemaFilePath);
  }

  void setGraphicsContext(std::unique_ptr<layer::SkiaGraphicsContext>& context, int w, int h)
  {
    m_context = std::move(context);

    m_context->init(m_graphicsContextConfig);
    m_appRender->init(m_context.get());

    m_mainComposer->view()->setSize(w, h);

    UEvent evt;
    evt.type = VGG_APP_INIT;
    evt.init.argc = 0;
    evt.init.argv = nullptr;
    evt.init.windowWidth = w;
    evt.init.windowHeight = h;

    const auto p = m_context->property();
    evt.init.drawableWidth = w * p.dpiScaling;
    evt.init.drawableHeight = h * p.dpiScaling;

    m_appRender->sendEvent(evt, nullptr);
  }

  bool run()
  {
    auto rendered = m_application->run(60);
    m_mainComposer->runLoop()->dispatch();
    return rendered;
  }
};

// api ----------------------------------------------------------------------
Component::Component()
  : m_impl(new ComponentImpl(this))
{
}

Component::~Component() = default;

bool Component::load(const std::string& filePath,
                     const char*        designDocSchemaFilePath,
                     const char*        layoutDocSchemaFilePath)
{

  return m_impl->load(filePath, designDocSchemaFilePath, layoutDocSchemaFilePath);
}

bool Component::run()
{
  return m_impl->run();
}

void Component::setGraphicsContext(std::unique_ptr<layer::SkiaGraphicsContext>& context,
                                   int                                          w,
                                   int                                          h)
{
  m_impl->setGraphicsContext(context, w, h);
}

} // namespace VGG