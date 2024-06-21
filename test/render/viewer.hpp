#pragma once
#include "Application/ZoomerNodeController.hpp"
#include "Domain/Model/Element.hpp"
#include "Layer/Core/DefaultResourceProvider.hpp"
#include "Layer/Core/RasterNode.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/ResourceManager.hpp"
#include "Layer/Model/JSONModel.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/Memory/Ref.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "loader.hpp"

#include "Layer/SceneBuilder.hpp"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/DocBuilder.hpp"
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Utility/ConfigManager.hpp"
#include "Application/Event/Event.hpp"
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

class Pager
{
  VGG::layer::SceneNode* m_sceneNode;
  int                    m_currentPage = -1;

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
  Pager(VGG::layer::SceneNode* sceneNode)
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

  VGG::Bounds getPageBounds()
  {
    if (m_currentPage < 0 || m_currentPage >= m_sceneNode->getFrames().size())
    {
      return VGG::Bounds();
    }
    const auto& frames = m_sceneNode->getFrames();
    return frames[m_currentPage]->bounds();
  }

  VGG::layer::FrameNode* currentFrame()
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

struct Viewer
{
  VGG::layer::Ref<VGG::layer::Viewport>   viewportNode;
  VGG::layer::Ref<VGG::layer::ZoomerNode> zoomNode;
  VGG::layer::Ref<VGG::layer::RasterNode> rasterNode;
  std::unique_ptr<Pager>                  pager;
  bool                                    panning{ false };
  std::function<void(int w, int h)>       viewportChangeCallback; // temporary solution

  // feature switcher
  bool enableDebug = false;
  bool enableDrawDamageBounds = false;
  bool enableHover = false;

  VGG::layer::PaintNode* focus = nullptr;
  bool                   hasNode = false;

  VGG::layer::PaintNode* selectedNode = nullptr;
  glm::ivec2             prevPos;

  static void paintNodeAt(
    VGG::layer::PaintNode*                      node,
    const VGG::layer::PaintNode::NodeAtContext* ctx)
  {
    auto that = static_cast<Viewer*>(ctx->userData);
    if (that->hasNode == false)
    {
      that->onNodeAt(node, ctx);
      that->hasNode = true;
    }
  }
  static void nodeAt(VGG::layer::RenderNode* node, const VGG::layer::RenderNode::NodeAtContext* ctx)
  {
    auto that = static_cast<Viewer*>(ctx->userData);
    static_cast<VGG::layer::FrameNode*>(node)->node()->nodeAt(
      ctx->localX,
      ctx->localY,
      paintNodeAt,
      that);
  }

  void sendNodeAt(int x, int y)
  {
    if (rasterNode)
    {
      rasterNode->nodeAt(x, y, nodeAt, this);
      if (!hasNode)
      {
        if (focus)
        {
          INFO("Hovering exit %s", focus->name().c_str());
#ifdef VGG_LAYER_DEBUG
          focus->hoverBounds = false;
#endif
          focus = nullptr;
        }
      }
      hasNode = false;
    }
  }

  void onNodeAt(VGG::layer::PaintNode* node, const VGG::layer::PaintNode::NodeAtContext* ctx)
  {
    if (node != focus)
    {
      INFO("Hovering enter in %s", node->name().c_str());
      if (focus)
      {
#ifdef VGG_LAYER_DEBUG
        focus->hoverBounds = false;
#endif
      }
#ifdef VGG_LAYER_DEBUG
      node->hoverBounds = true;
#endif
      focus = node;
    }
  }

  void onMouseTracking(int x, int y)
  {
    if (selectedNode)
    {
      auto offset = glm::ivec2(x, y) - prevPos;
      auto t = selectedNode->attributeAccessor()->getTransform();
      t.setMatrix(glm::translate(t.matrix(), glm::vec2(offset)));
      DEBUG("onMouseTracking %s, [%d, %d]", selectedNode->name().c_str(), offset.x, offset.y);
      selectedNode->attributeAccessor()->setTransform(t);
      prevPos = { x, y };
    }
  }

  bool onZoomEvent(UEvent e, void* userData)
  {
    if (
      !panning && e.type == VGG_MOUSEBUTTONDOWN &&
      (EventManager::getKeyboardState(nullptr)[VGG_SCANCODE_SPACE]))
    {
      panning = true;
      return true;
    }
    else if (panning && e.type == VGG_MOUSEBUTTONUP)
    {
      panning = false;
      return true;
    }
    else if (panning && e.type == VGG_MOUSEMOTION)
    {
      zoomNode->setTranslate(e.motion.xrel, e.motion.yrel);
      return true;
    }
    else if (e.type == VGG_MOUSEWHEEL && (EventManager::getModState() & VGG_KMOD_CTRL))
    {
      int mx = e.wheel.mouseX;
      int my = e.wheel.mouseY;
      if (auto l = zoomNode->discreteScaleLevel(); l)
      {
        zoomNode->setScale(
          layer::ZoomerNode::EScaleLevel(*l + int(e.wheel.preciseY > 0 ? 1 : -1)),
          { mx, my });
      }
      else
      {
        auto newScale = zoomNode->scale() + e.wheel.preciseY * 0.05;
        zoomNode->setScale(newScale, { mx, my });
      }
      return true;
    }
    return false;
  }

  bool onEvent(UEvent evt, void* userData)
  {
    // init events
    if (evt.type == VGG_APP_INIT)
    {
      return true;
    }

    // window event
    if (auto& window = evt.window;
        evt.type == VGG_WINDOWEVENT &&
        (window.event == VGG_WINDOWEVENT_RESIZED || window.event == VGG_WINDOWEVENT_SIZE_CHANGED))
    {
      if (viewportChangeCallback)
      {
        viewportChangeCallback(window.drawableWidth, window.drawableHeight);
      }
      viewportNode->setViewport(Bounds(0, 0, window.drawableWidth, window.drawableHeight));
      return true;
    }

    // camara event
    if (onZoomEvent(evt, userData))
      return true;

    // other events;

    if (evt.type == VGG_MOUSEMOTION)
    {
      if (enableHover)
      {
        sendNodeAt(evt.motion.windowX, evt.motion.windowY);
      }
      if (selectedNode)
      {
        onMouseTracking(evt.motion.windowX, evt.motion.windowY);
      }
    }

    if (evt.type == VGG_MOUSEBUTTONDOWN)
    {
      prevPos = { evt.motion.windowX, evt.motion.windowY };
      sendNodeAt(evt.motion.windowX, evt.motion.windowY);
      selectedNode = focus;
    }

    if (evt.type == VGG_MOUSEBUTTONUP)
    {
      selectedNode = nullptr;
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
      if (pager)
      {
        pager->prevFrame();
      }
      return true;
    }

    if (key == VGGK_PAGEDOWN && (mod & VGG_KMOD_CTRL))
    {
      INFO("Next page");
      if (pager)
      {
        pager->nextFrame();
      }
      return true;
    }

    if (key == VGGK_b)
    {
      enableDebug = !enableDebug;
      INFO("Toggle debug mode");
      return true;
    }

    if (key == VGGK_d)
    {
      enableDrawDamageBounds = !enableDrawDamageBounds;
      INFO("Toggle draw damage bounds");
      return true;
    }

    if (key == VGGK_h)
    {
      INFO("Toggle hover bound");
      enableHover = !enableHover;
      focus = nullptr;
      return true;
    }
    return false;
  }
};
