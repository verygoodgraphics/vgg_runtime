/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <argparse/argparse.hpp>
#include <algorithm>
#include <nlohmann/json.hpp>

#include "Entity/EntityManager.hpp"
#include "Entity/InputManager.hpp"
#include "Systems/RenderSystem.hpp"
#include "Utils/App.hpp"
#include "Utils/FileManager.hpp"
#include "Utils/Version.hpp"
#include "Utils/Scheduler.hpp"

using namespace VGG;

class Runtime : public App
{
  struct Zoomer
  {
    static constexpr double minZoom = 0.01;
    static constexpr double maxZoom = 10.0;
    Vec2 offset{ 0.0, 0.0 };
    double zoom{ 1.0 };
    bool panning{ false };

    void apply(SkCanvas* canvas)
    {
      ASSERT(canvas);
      canvas->translate(offset.x, offset.y);
      canvas->scale(zoom, zoom);
    }

    void restore(SkCanvas* canvas)
    {
      ASSERT(canvas);
      canvas->scale(1. / zoom, 1. / zoom);
      canvas->translate(-offset.x, -offset.y);
    }

    SDL_Event mapEvent(SDL_Event evt, double scaleFactor)
    {
      if (evt.type == SDL_MOUSEMOTION)
      {
        double x1 = evt.motion.x;
        double y1 = evt.motion.y;
        double x0 = x1 - evt.motion.xrel;
        double y0 = y1 - evt.motion.yrel;
        x1 = (x1 / scaleFactor - offset.x) / zoom;
        y1 = (y1 / scaleFactor - offset.y) / zoom;
        x0 = (x0 / scaleFactor - offset.x) / zoom;
        y0 = (y0 / scaleFactor - offset.y) / zoom;
        evt.motion.x = FLOAT2SINT(x1);
        evt.motion.y = FLOAT2SINT(y1);
        evt.motion.xrel = FLOAT2SINT(x1 - x0);
        evt.motion.yrel = FLOAT2SINT(y1 - y0);
      }
      return evt;
    }
  };
  Zoomer m_zoomer;

public:
  inline void setOnFrameOnce(std::function<void()>&& cb)
  {
    Scheduler::setOnFrameOnce(std::move(cb));
  }

  inline void startRunMode()
  {
    EntityManager::setInteractionMode(EntityManager::InteractionMode::RUN);
    InputManager::setMouseCursor(MouseEntity::CursorType::NORMAL);
    if (auto container = EntityManager::getEntities())
    {
      container->setRunModeInteractions();
    }
  }

protected:
  virtual void onInit() override
  {
    SDL_ShowCursor(SDL_DISABLE);
  }

  virtual void onFrame() override
  {
    Scheduler::callOnFrameOnce();

    if (SkCanvas* canvas = getCanvas())
    {
      m_zoomer.apply(canvas);
      EntityManager::map([&](Entity& entity) { RenderSystem::drawEntity(canvas, entity); });
      InputManager::draw(canvas);
      m_zoomer.restore(canvas);
    }

    InputManager::onFrame();
  }

  virtual void onEvent(const SDL_Event& evt) override
  {
    auto e = m_zoomer.mapEvent(evt, DPI::ScaleFactor);
    InputManager::onEvent(e);
  }

  virtual bool onGlobalEvent(const SDL_Event& evt) override
  {
    auto type = evt.type;

    auto& panning = m_zoomer.panning;
    if (!panning && type == SDL_MOUSEBUTTONDOWN && (SDL_GetModState() & KMOD_CTRL))
    {
      panning = true;
      InputManager::setMouseCursor(MouseEntity::CursorType::MOVE);
      return true;
    }
    else if (panning && type == SDL_MOUSEBUTTONUP)
    {
      panning = false;
      InputManager::setMouseCursor(MouseEntity::CursorType::NORMAL);
      return true;
    }
    else if (panning && type == SDL_MOUSEMOTION)
    {
      m_zoomer.offset.x += evt.motion.xrel / DPI::ScaleFactor;
      m_zoomer.offset.y += evt.motion.yrel / DPI::ScaleFactor;
      return true;
    }
    else if (type == SDL_MOUSEWHEEL && (SDL_GetModState() & KMOD_CTRL))
    {
      int mx, my;
      SDL_GetMouseState(&mx, &my);
      double dz = (evt.wheel.y > 0 ? 1.0 : -1.0) * 0.03;
      double z2 = m_zoomer.zoom * (1 + dz);
      if (z2 > 0.01 && z2 < 100)
      {
        m_zoomer.offset.x -= (mx / DPI::ScaleFactor - m_zoomer.offset.x) * dz;
        m_zoomer.offset.y -= (my / DPI::ScaleFactor - m_zoomer.offset.y) * dz;
        m_zoomer.zoom += m_zoomer.zoom * dz;
        return true;
      }
      return false;
    }
    if (type == SDL_KEYDOWN)
    {
      auto key = evt.key.keysym.sym;
      auto mod = evt.key.keysym.mod;

      if (key == SDLK_PAGEUP)
      {
        FileManager::prevPage();
        return true;
      }

      if (key == SDLK_PAGEDOWN)
      {
        FileManager::nextPage();
        return true;
      }

#ifndef EMSCRIPTEN
      if ((mod & KMOD_CTRL) && key == SDLK_q)
      {
        m_shouldExit = true;
        return true;
      }
#endif
    }

    return false;
  }
};

#ifdef EMSCRIPTEN
extern "C"
{
  void emscripten_frame()
  {
    static Runtime* app = App::getInstance<Runtime>();
    ASSERT(app);
    app->frame();
  }
  void emscripten_main(int width, int height)
  {
    Runtime* app = App::getInstance<Runtime>(width, height);
    ASSERT(app);
    if (auto fm = FileManager::getInstance(); fm && fm->fileCount() < 1)
    {
      FileManager::newFile();
    }
    app->setOnFrameOnce([app]() { app->startRunMode(); });
    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
#else
int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", Version::get());
  program.add_argument("-l", "--load").help("load from vgg or sketch file");
  program.add_argument("-c", "--convert")
    .help("convert sketch to vgg file")
    .default_value(false)
    .implicit_value(true);

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

  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();
    auto ext = FileManager::getLoweredFileExt(fp);
    auto convert = program.get<bool>("--convert");

    if (convert && ext != "sketch")
    {
      FAIL("Cannot convert non-sketch file: %s", fp.c_str());
      exit(1);
    }

    if (!FileManager::loadFile(fp))
    {
      FAIL("Failed to load file: %s", fp.c_str());
      exit(1);
    }

    if (convert && ext == "sketch")
    {
      auto dir = std::filesystem::current_path();
      auto name = FileManager::getFileName(fp);
      auto targetFp = dir / (name + ".vgg");
      if (FileManager::saveFileAs(0, targetFp))
      {
        INFO("Conversion succeeded: %s", targetFp.c_str());
        exit(0);
      }
      else
      {
        FAIL("Conversion failed for: %s", fp.c_str());
        exit(1);
      }
    }
  }

  Runtime* app = App::getInstance<Runtime>(1200, 800, "VGG");
  ASSERT(app);
  if (auto fm = FileManager::getInstance(); fm && fm->fileCount() < 1)
  {
    FileManager::newFile();
  }

  // enter run mode
  app->setOnFrameOnce([app]() { app->startRunMode(); });

  // enter loop
  while (!app->shouldExit())
  {
    app->frame();
  }
  return 0;
}
#endif
