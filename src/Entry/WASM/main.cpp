#include "Entry/SDL/SDLRuntime.hpp"
#include "BrowserMainComposer.hpp"
#include <ConfigMananger.h>

#include <memory>
extern "C"
{
  void emscripten_frame()
  {
    static SDLRuntime* app = App<SDLRuntime>::getInstance();
    ASSERT(app);
    constexpr int FPS = 60;
    app->frame(FPS);

    auto& mainComposer = VggBrowser::mainComposer();
    mainComposer.runLoop()->dispatch();
  }
  void emscripten_main(int width, int height)
  {
    Config::readGlobalConfig("/asset/etc/config.json");

    SDLRuntime* app = App<SDLRuntime>::getInstance(width, height);
    ASSERT(app);

    auto& mainComposer = VggBrowser::mainComposer();
    app->setController(mainComposer.controller());
    app->setView(mainComposer.view());
    app->setScene(mainComposer.view()->scene());

    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
