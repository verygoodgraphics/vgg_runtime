#pragma once

#include "loader.hpp"
#include "Entry/Common/DocBuilder.hpp"
#include "Utility/ConfigManager.hpp"
#include "Application/AppRender.hpp"
#include "Application/AppRenderable.hpp"
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Application/Event/Event.hpp"
#include "Application/Event/EventListener.hpp"
#include "Application/Event/Keycode.hpp"

#include "Layer/Scene.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/VGGLayer.hpp"
#include "Layer/Exporter/SVGExporter.hpp"
#include "Layer/Exporter/PDFExporter.hpp"

#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <argparse/argparse.hpp>
#include <filesystem>
#include <filesystem>
namespace fs = std::filesystem;

using namespace VGG::app;

class SkCanvas;
class Editor : public VGG::app::AppRenderable
{
public:
  Editor()
  {
  }

  bool onEvent(UEvent e, void* userData) override
  {
    // Handle editor code herer
    return true;
  }

protected:
  void onRender(SkCanvas* canvas) override
  {
    // drawing here
  }
};

constexpr char POS_ARG_INPUT_FILE[] = "fig/ai/sketch/json";
template<typename App>
class MyEventListener : public VGG::app::EventListener
{
  AppRender*                m_layer{ nullptr };
  std::shared_ptr<AppScene> m_scene;

protected:
  void onAppInit(App* app, char** argv, int argc)
  {
    INFO("onAppInit");
    m_layer = app->layer();
    argparse::ArgumentParser program("vgg", "0.1");
    program.add_argument(POS_ARG_INPUT_FILE).help("input fig/ai/sketch/json file");
    program.add_argument("-d", "--data").help("resources dir");
    program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
    program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");
    program.add_argument("-c", "--config").help("specify config file");
    program.add_argument("-w", "--width")
      .help("width of viewport")
      .scan<'i', int>()
      .default_value(1200);
    program.add_argument("-h", "--height")
      .help("height of viewport")
      .scan<'i', int>()
      .default_value(800);

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

    m_scene = std::make_shared<AppScene>();
    m_scene->setZoomerListener(std::make_shared<AppZoomer>());

    std::map<std::string, std::vector<char>> resources;
    std::filesystem::path                    prefix;
    std::filesystem::path                    respath;

    if (auto p = program.present("-p"))
    {
      prefix = p.value();
    }
    if (auto loadfile = program.present(POS_ARG_INPUT_FILE))
    {
      auto fp = loadfile.value();
      auto ext = fs::path(fp).extension().string();
      if (ext == ".json")
      {
        respath = std::filesystem::path(fp).stem(); // same with filename as default
        if (auto res = program.present("-d"))
        {
          respath = res.value();
        }
      }

      auto r = load(ext);
      if (r)
      {
        auto data = r->read(prefix / fp);
        Scene::setResRepo(data.resource);
        try
        {
          auto res = VGG::entry::DocBuilder::builder()
                       .setDocument(std::move(data.format))
                       .setLayout(std::move(data.layout))
                       .setExpandEnabled(true)
                       .setLayoutEnabled(true)
                       .build();
          INFO("Preprocess Time Cost: ");
          std::cout << res.info;
          auto dur = Timer::time([&, this]() { m_scene->loadFileContent(*res.doc); });
          INFO("Scene Build Time Cost: %f", (double)dur.s());
          auto editor = std::make_shared<Editor>();
          m_layer->addAppRenderable(editor);
          m_layer->addAppScene(m_scene);
          m_layer->setDrawPositionEnabled(true);
        }
        catch (std::exception& e)
        {
          FAIL("load json failed: %s", e.what());
        }
      }
    }
  }

  void onAppExit()
  {
    INFO("Exit...");
  }

public:
  bool onEvent(UEvent evt, void* userData) override
  {
    if (evt.type == VGG_APP_INIT)
    {
      auto app = reinterpret_cast<App*>(userData);
      onAppInit(app, evt.init.argv, evt.init.argc);
      return true;
    }
    if (evt.type != VGG_KEYDOWN)
    {
      return false;
    }
    auto type = evt.type;
    auto key = evt.key.keysym.sym;
    auto mod = evt.key.keysym.mod;

    if (key == VGGK_PAGEUP && (mod & VGG_KMOD_CTRL))
    {
      INFO("Previous page");
      if (m_scene)
      {
        m_scene->preArtboard();
      }
      return true;
    }

    if (key == VGGK_PAGEDOWN && (mod & VGG_KMOD_CTRL))
    {
      INFO("Next page");
      if (m_scene)
      {
        m_scene->nextArtboard();
      }
      return true;
    }

    if (key == VGGK_1)
    {
      INFO("Capture SKP");
      std::ofstream ofs("capture.skp");
      if (ofs.is_open())
      {
        m_layer->makeSKP(ofs);
      }
      return true;
    }

    if (key == VGGK_2)
    {
      INFO("Capture SVG");
      std::ofstream ofs("capture.svg");
      if (ofs.is_open())
      {
        using namespace VGG::layer::exporter;
        auto                 page = m_scene->currentPage();
        auto                 f = m_scene->frame(page);
        auto                 b = f->bound();
        exporter::SVGOptions opt;
        opt.extend[0] = b.width();
        opt.extend[1] = b.height();
        makeSVG(m_scene.get(), opt, ofs);
      }
      return true;
    }

    if (key == VGGK_3)
    {
      INFO("Capture PNG");
      std::ofstream       ofs("capture.png");
      layer::ImageOptions opt;
      auto                f = m_scene->frame(m_scene->currentPage());
      auto                b = f->bound();
      opt.extend[0] = b.width();
      opt.extend[1] = b.height();
      opt.encode = VGG::layer::EImageEncode::IE_PNG;
      if (ofs.is_open())
      {
        m_layer->makeImageSnapshot(opt, ofs);
      }
      return true;
    }

    if (key == VGGK_4)
    {
      INFO("Capture JPEG");
      std::ofstream       ofs("capture.jpg");
      layer::ImageOptions opt;
      auto                f = m_scene->frame(m_scene->currentPage());
      auto                b = f->bound();
      opt.extend[0] = b.width();
      opt.extend[1] = b.height();
      opt.encode = VGG::layer::EImageEncode::IE_JPEG;
      if (ofs.is_open())
      {
        m_layer->makeImageSnapshot(opt, ofs);
      }
      return true;
    }

    if (key == VGGK_5)
    {
      INFO("Capture WEBP");
      std::ofstream       ofs("capture.webp");
      layer::ImageOptions opt;
      auto                f = m_scene->frame(m_scene->currentPage());
      auto                b = f->bound();
      opt.extend[0] = b.width();
      opt.extend[1] = b.height();
      opt.encode = VGG::layer::EImageEncode::IE_WEBP;
      if (ofs.is_open())
      {
        m_layer->makeImageSnapshot(opt, ofs);
      }
      return true;
    }

    if (key == VGGK_6)
    {
      INFO("Capture PDF");
      std::ofstream ofs("capture.pdf");
      if (ofs.is_open())
      {
        using namespace VGG::layer::exporter;
        auto                 page = m_scene->currentPage();
        auto                 f = m_scene->frame(page);
        auto                 b = f->bound();
        exporter::PDFOptions opt;
        opt.extend[0] = b.width();
        opt.extend[1] = b.height();
        makePDF(m_scene.get(), opt, ofs);
      }
      return true;
    }

    if (key == VGGK_b)
    {
      INFO("Toggle object bounding box");
      m_scene->enableDrawDebugBound(!m_scene->isEnableDrawDebugBound());
      return true;
    }

    if (key == VGGK_1)
    {
      INFO("Toggle cursor position");
      m_layer->setDrawPositionEnabled(!m_layer->enableDrawPosition());
      return true;
    }

    if (key == VGGK_s)
    {
      return true;
    }
    return false;
  }
};
