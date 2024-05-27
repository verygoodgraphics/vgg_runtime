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

#include <stdlib.h>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include "Adapter/Environment.hpp"
#include "Adapter/NativeComposer.hpp"
#include "AppSDLImpl.hpp"
#include "Application/AppBase.hpp"
#include "Application/Controller.hpp"
#include "Application/Event/EventListener.hpp"
#include "Application/MainComposer.hpp"
#include "Application/RunLoop.hpp"
#include "Application/UIApplication.hpp"
#include "Application/VggEnv.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"
#include "SdlMouse.hpp"
#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"
#include "Utility/Version.hpp"
#include <argparse/argparse.hpp>

constexpr auto DARUMA_FILE_OR_DIRECTORY = "daruma";

using namespace VGG;
using namespace VGG::app;

using AppImpl = VGG::entry::AppSDLImpl;

int main(int argc, char** argv)
{
  INFO("#vgg runtime version: %s", VGG::Version::get().c_str());

  VGG::Environment::setUp();

  argparse::ArgumentParser program("vgg", "0.1");
  program.add_argument(DARUMA_FILE_OR_DIRECTORY).help("daruma file or directory");
  program.add_argument("-c", "--config").help("specify config file");

  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err)
  {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }

  if (auto configfile = program.present("-c"))
  {
    auto file = configfile.value();
    Config::readGlobalConfig(file);
  }

  AppConfig cfg;
  cfg.appName = "SdlRuntime";
  cfg.windowSize[0] = 1920;
  cfg.windowSize[1] = 1080;
  cfg.graphicsContextConfig.multiSample = 4;
  cfg.graphicsContextConfig.stencilBit = 8;
  cfg.argc = argc;
  cfg.argv = argv;

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

  bool catchJsException = false;
#ifdef NDEBUG
  catchJsException = true;
#endif
  MainComposer mainComposer{ new NativeComposer(catchJsException), std::make_shared<SdlMouse>() };
  mainComposer.env()->setApplication(app);

  auto sdlApp = AppImpl::app();

  // inject dependencies
  app->setLayer(sdlApp->layer()); // 1. must be first
  auto view = mainComposer.view();
  app->setView(view, cfg.windowSize[0], cfg.windowSize[1]);

  auto controller = mainComposer.controller();
  // controller->setEditMode(true);
  // controller->setFitToViewportEnabled(false);
  // controller->setContentMode("scaleAspectFill");
  // controller->setContentMode("scaleAspectFit");
  auto darumaFileOrDir = program.get<std::string>(DARUMA_FILE_OR_DIRECTORY);
  controller->start(darumaFileOrDir, "../asset/vgg-format.json", "../asset/vgg_layout.json");
  app->setController(controller);

  while (!sdlApp->shouldExit())
  {
    sdlApp->poll();
    app->paint(cfg.renderFPSLimit);
    mainComposer.runLoop()->dispatch();
  }

  VGG::Environment::tearDown();
  return 0;
}
