#include "Entry/SDL/SDLRuntime.hpp"
#include "Utils/FileManager.hpp"
#include "Main/MainComposer.hpp"
#include <memory>
extern "C"
{
  void emscripten_frame()
  {
    static SDLRuntime* app = App<SDLRuntime>::getInstance();
    ASSERT(app);
    constexpr int fps = 60;
    app->frame(fps);

    auto& main_composer = MainComposer::instance();
    main_composer.runLoop()->dispatch();
  }
  void emscripten_main(int width, int height)
  {
    SDLRuntime* app = App<SDLRuntime>::getInstance(width, height);
    ASSERT(app);
    if (auto fm = FileManager::getInstance(); fm && fm->fileCount() < 1)
    {
      FileManager::newFile();
    }

    auto& main_composer = MainComposer::instance();
    app->setView(main_composer.view());
    app->setScene(main_composer.view()->scene());

    app->setOnFrameOnce([app]() { app->startRunMode(); });
    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
