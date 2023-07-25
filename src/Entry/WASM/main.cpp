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
    constexpr int fps = 60;
    app->frame(fps);

    auto& main_composer = VggBrowser::mainComposer();
    main_composer.runLoop()->dispatch();
  }
  void emscripten_main(int width, int height)
  {
    Config::readGlobalConfig("/asset/etc/config.json");

    SDLRuntime* app = App<SDLRuntime>::getInstance(width, height);
    ASSERT(app);

    auto& main_composer = VggBrowser::mainComposer();
    app->setView(main_composer.view());
    app->setScene(main_composer.view()->scene());

    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
