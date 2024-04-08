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
#include "Entry/SDL/AppSDLImpl.hpp"
#include "BrowserMainComposer.hpp"

#include "Application/Presenter.hpp"
#include "Application/RunLoop.hpp"
#include "Application/UIApplication.hpp"
#include "Layer/FontManager.hpp"
#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"
#include "Utility/Version.hpp"

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include <memory>

using AppImpl = VGG::entry::AppSDLImpl;

namespace
{
UIApplication* application()
{
  static auto s_app = new UIApplication;
  return s_app;
}

void vggEmscriptenFrame()
{
  static auto s_sdlApp = AppImpl::app();
  ASSERT(s_sdlApp);

  s_sdlApp->poll();

  application()->paint(s_sdlApp->appConfig().renderFPSLimit);

  auto& mainComposer = VggBrowser::mainComposer();
  mainComposer.runLoop()->dispatch();
}

class VggWasm
{
  bool m_canceled = false;

public:
  static VggWasm& instance()
  {
    static VggWasm s_instance;
    return s_instance;
  }

  bool isCanceled() const
  {
    return m_canceled;
  }

  void startMainLoop()
  {
    cancelMainLoop(); // cancel first; emscripten_set_main_loop should be called only ONCE

    m_canceled = false;
    // pass 0/false as the third argument
    // https://github.com/emscripten-core/emscripten/issues/16071
    emscripten_set_main_loop(vggEmscriptenFrame, 0, 0);
  }

  void cancelMainLoop()
  {
    m_canceled = true;
    emscripten_cancel_main_loop();
  }

  void exit()
  {
    cancelMainLoop();
    emscripten_force_exit(0);
  }
};
} // namespace

extern "C"
{

  void emscripten_main(int width, int height, bool editMode) // NOLINT
  {
    Config::readGlobalConfig("/asset/etc/config.json");

    VGG::app::AppConfig cfg;
    cfg.appName = "SdlRuntime";
    cfg.windowSize[0] = width;
    cfg.windowSize[1] = height;
    cfg.graphicsContextConfig.multiSample = 1;
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
    mainComposer.env()->setApplication(app);

    // inject dependencies
    app->setLayer(sdlApp->layer());

    auto view = mainComposer.view();
    app->setView(view, cfg.windowSize[0], cfg.windowSize[1]);

    auto controller = mainComposer.controller();
    controller->setEditMode(editMode);
    app->setController(controller);
  }

  EMSCRIPTEN_KEEPALIVE void vggExit()
  {
    VggWasm::instance().exit();
  }

  bool load_file_from_mem(const char* name, char* data, int len)
  {
    std::vector<char> buf(data, data + len);

    auto controller = VggBrowser::mainComposer().controller();
    auto ret = controller->start(buf, "/asset/vgg-format.json", "/asset/vgg_layout.json");

    if (VggWasm::instance().isCanceled())
    {
      WARN("Vgg main loop is canceled, return");
      return false;
    }

    VggWasm::instance().startMainLoop();

    return ret;
  }

  bool is_latest_version(const char* version)
  {
    std::string v1(version);
    std::string v2 = VGG::Version::get();
    return v1 == v2;
  }

  EMSCRIPTEN_KEEPALIVE void listenAllEvents(bool enabled)
  {
    auto& mainComposer = VggBrowser::mainComposer();

    return mainComposer.controller()->listenAllEvents(enabled);
  }
}
