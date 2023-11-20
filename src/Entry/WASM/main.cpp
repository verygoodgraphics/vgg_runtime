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
#include "Entry/SDL/AppSDLImpl.hpp"
#include "BrowserMainComposer.hpp"

#include "Application/RunLoop.hpp"
#include "Application/UIApplication.hpp"
#include "Application/UIView.hpp"
#include "Utility/ConfigManager.hpp"

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include <memory>

using AppImpl = VGG::entry::AppSDLImpl;

UIApplication* application()
{
  static auto s_app = new UIApplication;
  return s_app;
}

extern "C"
{
  void emscripten_frame() // NOLINT
  {
    static auto s_sdlApp = AppImpl::app();
    ASSERT(s_sdlApp);

    s_sdlApp->poll();

    application()->run(s_sdlApp->appConfig().renderFPSLimit);

    auto& mainComposer = VggBrowser::mainComposer();
    mainComposer.runLoop()->dispatch();
  }

  void emscripten_main(int width, int height, bool editMode) // NOLINT
  {
    Config::readGlobalConfig("/asset/etc/config.json");

    VGG::app::AppConfig cfg;
    cfg.appName = "SdlRuntime";
    cfg.windowSize[0] = width;
    cfg.windowSize[1] = height;
    cfg.graphicsContextConfig.multiSample = 4;
    cfg.graphicsContextConfig.stencilBit = 8;

    auto app = application();
    cfg.eventListener.reset(app);

    try
    {
      AppImpl::createInstance(std::move(cfg));
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }

    auto sdlApp = AppImpl::app();
    ASSERT(sdlApp);
    auto& mainComposer = VggBrowser::mainComposer();

    // inject dependencies
    app->setLayer(sdlApp->layer());

    auto view = mainComposer.view();
    view->setSize(cfg.windowSize[0], cfg.windowSize[1]);
    app->setView(view);

    auto controller = mainComposer.controller();
    controller->setEditMode(editMode);
    app->setController(controller);

    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
