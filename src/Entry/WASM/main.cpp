#include "Entry/SDL/SDLImpl/AppSDLImpl.hpp"
#include "BrowserMainComposer.hpp"

#include "Application/UIApplication.hpp"
#include "ConfigMananger.h"

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

  void emscripten_main(int width, int height) // NOLINT
  {
    Config::readGlobalConfig("/asset/etc/config.json");

    VGG::app::AppConfig cfg;
    cfg.appName = "SdlRuntime";
    cfg.windowSize[0] = width;
    cfg.windowSize[1] = height;
    cfg.graphicsContextConfig.multiSample = 0;
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
    app->setView(mainComposer.view());
    app->setController(mainComposer.controller());

    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
