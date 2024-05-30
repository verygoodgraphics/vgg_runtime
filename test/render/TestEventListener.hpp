#pragma once

#include "Application/ZoomerNodeController.hpp"
#include "Domain/Model/Element.hpp"
#include "Layer/Core/DefaultResourceProvider.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/ResourceManager.hpp"
#include "Layer/Model/JSONModel.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/Memory/Ref.hpp"
#include "loader.hpp"

#include "Layer/SceneBuilder.hpp"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/DocBuilder.hpp"
#include "Layer/Core/RasterCacheTile.hpp"
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Utility/ConfigManager.hpp"
#include "Application/AppRender.hpp"
#include "Application/AppRenderable.hpp"
#include "Application/Event/Event.hpp"
#include "Application/Event/EventListener.hpp"
#include "Application/Event/Keycode.hpp"

#include "Layer/Core/PaintNode.hpp"
#include "Layer/VGGLayer.hpp"
#include "Layer/Exporter/SVGExporter.hpp"
#include "Layer/Exporter/PDFExporter.hpp"
#include "Layer/Config.hpp"

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

struct KeyboardState
{
public:
};

struct MouseState
{
};

class Action
{
public:
  virtual void match(const KeyboardState& state) = 0;
  virtual void operator()(void* userData) = 0;
};

void displayInfo(VGG::layer::PaintNode* node)
{
  if (node)
  {
    INFO("Name: %s", node->name().c_str());
    INFO("ID: %s", node->guid().c_str());
    const auto bounds = node->bounds();
    INFO(
      "Bounds: [%f, %f, %f, %f]",
      bounds.topLeft().x,
      bounds.topLeft().y,
      bounds.width(),
      bounds.height());
  }
}

class Pager
{
  layer::SceneNode* m_sceneNode;
  int               m_currentPage = -1;

  void setPageOffset(int delta)
  {
    if (m_sceneNode && !m_sceneNode->getFrames().empty())
    {
      const auto total = m_sceneNode->getFrames().size();
      auto       newPage = (m_currentPage + delta + total) % total;
      if (newPage == m_currentPage)
        return;
      const auto& frames = m_sceneNode->getFrames();
      frames[m_currentPage]->node()->setVisible(false);
      INFO("Set page %d", (int)newPage);
      frames[newPage]->node()->setVisible(true);
      m_currentPage = newPage;
    }
  }

public:
  Pager(layer::SceneNode* sceneNode)
    : m_sceneNode(sceneNode)
  {
    if (m_sceneNode && !m_sceneNode->getFrames().empty())
    {
      const auto& frames = m_sceneNode->getFrames();
      for (int i = 0; i < m_sceneNode->getFrames().size(); i++)
      {
        frames[i]->node()->setVisible(false);
      }
      m_currentPage = 0;
      frames[m_currentPage]->node()->setVisible(true);
    }
    else
    {
      m_currentPage = -1;
    }
  }

  Bounds getPageBounds()
  {
    if (m_currentPage < 0 || m_currentPage >= m_sceneNode->getFrames().size())
    {
      return Bounds();
    }
    const auto& frames = m_sceneNode->getFrames();
    return frames[m_currentPage]->bounds();
  }

  layer::FrameNode* currentFrame()
  {
    if (m_currentPage < 0 || m_currentPage >= m_sceneNode->getFrames().size())
    {
      return nullptr;
    }
    return m_sceneNode->getFrames()[m_currentPage].get();
  }

  void setPage(int index)
  {
    setPageOffset(index - m_currentPage);
  }

  void nextFrame()
  {
    setPageOffset(1);
  }

  void prevFrame()
  {
    setPageOffset(-1);
  }
};

constexpr char POS_ARG_INPUT_FILE[] = "fig/ai/sketch/json";
template<typename App>
class MyEventListener : public VGG::app::EventListener
{
  AppRender*                               m_layer{ nullptr };
  layer::Ref<layer::SceneNode>             m_sceneNode;
  std::unique_ptr<Pager>                   m_pager;
  std::unique_ptr<app::ZoomNodeController> m_zoomController;

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
    program.add_argument("-m", "--model").help("model read by").default_value("struct");
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

    std::filesystem::path prefix;
    std::filesystem::path respath;

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

        VGG::layer::setGlobalResourceProvider(std::move(data.provider));
        try
        {
          auto res = VGG::entry::DocBuilder::builder()
                       .setDocument(std::move(data.format))
                       .setLayout(std::move(data.layout))
                       .setExpandEnabled(true)
                       .setLayoutEnabled(true)
                       .build();

          layer::SceneBuilderResult sceneBuilderResult;
          layer::Timer              t;
          if (program.get("-m") == "json")
          {
            std::vector<layer::JSONFrameObject> frames;
            std::vector<nlohmann::json>         jsonModels;
            for (auto& e : *res.doc)
            {
              if (e->type() == VGG::Domain::Element::EType::FRAME)
              {
                auto f = static_cast<VGG::Domain::FrameElement*>(e.get());
                jsonModels.emplace_back(f->treeModel(true));
                frames.emplace_back(layer::JSONFrameObject(jsonModels.back()));
              }
            }
            t.start();
            sceneBuilderResult = VGG::layer::SceneBuilder::builder()
                                   .setResetOriginEnable(true)
                                   .setAllocator(layer::getGlobalMemoryAllocator())
                                   .build<layer::JSONModelFrame>(std::move(frames));
            t.stop();
          }
          else if (program.get("-m") == "struct")
          {
            std::vector<layer::StructFrameObject> frames;
            for (auto& f : *res.doc)
            {
              if (f->type() == VGG::Domain::Element::EType::FRAME)
              {
                frames.emplace_back(layer::StructFrameObject(f.get()));
              }
            }
            t.start();
            sceneBuilderResult = VGG::layer::SceneBuilder::builder()
                                   .setResetOriginEnable(true)
                                   .setAllocator(layer::getGlobalMemoryAllocator())
                                   .build<layer::StructModelFrame>(std::move(frames));
            t.stop();
          }
          auto dur = t.elapsed();
          INFO("Doc Expand Time Cost: %f", (double)res.timeCost.expand.s());
          INFO("Doc Layout Time Cost: %f", (double)res.timeCost.layout.s());
          INFO("Scene Build Time Cost: %f", (double)dur.s());
          if (sceneBuilderResult.type)
          {
            if (
              *sceneBuilderResult.type ==
              VGG::layer::SceneBuilderResult::EResultType::VERSION_MISMATCH)
            {
              DEBUG("Format Version mismatch:");
            }
          }
          if (sceneBuilderResult.root)
          {
            m_sceneNode = layer::SceneNode::Make(std::move(*sceneBuilderResult.root));
            auto zoomNode = layer::ZoomerNode::Make();
            m_zoomController = std::make_unique<ZoomNodeController>(zoomNode);
            if (false)
            {
              m_layer->setRenderNode(std::move(zoomNode), m_sceneNode);
            }
            else
            {
              m_layer->setRenderNode(layer::TransformEffectNode::Make(zoomNode, m_sceneNode));
            }
            m_pager = std::make_unique<Pager>(m_sceneNode.get());
          }
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
    if (m_zoomController)
    {
      if (m_zoomController->onEvent(evt, userData))
      {
        return true;
      }
    }

    static bool                   s_enableHover = false;
    static VGG::layer::PaintNode* s_currentHover = nullptr;
    if (evt.type == VGG_MOUSEMOTION)
    {
#ifdef VGG_LAYER_DEBUG
      if (m_layer && s_enableHover)
      {
        if (auto p = m_layer->nodeAt(evt.motion.windowX, evt.motion.windowY); p)
        {
          // enter
          if (p != s_currentHover)
          {
            INFO("Hovering enter in %s", p->name().c_str());
            if (s_currentHover)
            {
              s_currentHover->hoverBounds = false;
            }
            p->hoverBounds = true;
            s_currentHover = p;
            displayInfo(p);
          }
        }
        else
        {
          // exit
          if (s_currentHover)
          {
            INFO("Hovering exit %s", s_currentHover->name().c_str());
            s_currentHover->hoverBounds = false;
            s_currentHover = nullptr;
          }
        }
      }
#endif
    }

    if (evt.type == VGG_MOUSEBUTTONDOWN)
    {
#ifdef VGG_LAYER_DEBUG
      if (m_layer)
      {
        if (auto p = m_layer->nodeAt(evt.motion.windowX, evt.motion.windowY); p)
        {
          INFO("%s", p->dump().c_str());
        }
      }
#endif
    }

    // keyboard event
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
      if (m_pager)
      {
        m_pager->prevFrame();
      }
      return true;
    }

    if (key == VGGK_PAGEDOWN && (mod & VGG_KMOD_CTRL))
    {
      INFO("Next page");
      if (m_pager)
      {
        m_pager->nextFrame();
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
        auto f = m_pager->currentFrame();
        f->revalidate();
        auto                        b = f->bounds();
        layer::exporter::SVGOptions opt;
        opt.extend[0] = b.width();
        opt.extend[1] = b.height();
        makeSVG(f, opt, ofs);
      }
      return true;
    }

    if (key == VGGK_3)
    {
      std::ofstream       ofs("capture.png");
      layer::ImageOptions opt;
      auto                b = m_pager->getPageBounds();
      opt.extend[0] = b.width();
      opt.extend[1] = b.height();
      INFO("Capture PNG [%f, %f]", b.width(), b.height());
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
      auto                b = m_pager->getPageBounds();
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
      auto                b = m_pager->getPageBounds();
      opt.extend[0] = b.width();
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
        auto f = m_pager->currentFrame();
        f->revalidate();
        auto b = f->bounds();

        layer::exporter::PDFOptions opt;
        opt.extend[0] = b.width();
        opt.extend[1] = b.height();
        makePDF(f, opt, ofs);
      }
      return true;
    }

    if (key == VGGK_h)
    {
      INFO("Toggle hover bound");
      s_enableHover = !s_enableHover;
      return true;
    }

    if (key == VGGK_b)
    {
      INFO("Toggle debug mode");
      m_layer->setDebugModeEnabled(!m_layer->debugModeEnabled());
      return true;
    }

    if (key == VGGK_s)
    {
      return true;
    }
    return false;
  }
};
