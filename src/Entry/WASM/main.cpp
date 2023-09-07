#include "Entry/SDL/SDLImpl/AppSDLImpl.hpp"
#include "BrowserMainComposer.hpp"
#include <ConfigMananger.h>

#include <memory>
extern "C"
{
  void emscripten_frame()
  {
    static auto sdlApp = AppImpl::app();
    ASSERT(sdlApp);

    sdlApp->poll();
    sdlApp->process();

    auto& mainComposer = VggBrowser::mainComposer();
    mainComposer.runLoop()->dispatch();
  }

  void emscripten_main(int width, int height)
  {
    Config::readGlobalConfig("/asset/etc/config.json");

    AppConfig cfg;
    cfg.appName = "SdlRuntime";
    cfg.graphicsContextConfig.windowSize[0] = width;
    cfg.graphicsContextConfig.windowSize[1] = height;
    cfg.graphicsContextConfig.multiSample = 0;
    cfg.graphicsContextConfig.stencilBit = 8;

    auto app = new UIApplication;
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
    app->setScene(mainComposer.view());
    app->setController(mainComposer.controller());

    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
