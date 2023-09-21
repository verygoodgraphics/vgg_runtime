#pragma once

#include "Application/AppRender.h"
#include "Application/Controller.hpp"
#include "Application/UIView.hpp"
#include "Utility/ConfigMananger.h"
#include "Event/Event.h"
#include "Event/EventListener.h"
#include "Event/Keycode.h"

#include "VGG/Layer/Scene.h"
#include "VGG/Layer/VGGLayer.h"

#include <nlohmann/json.hpp>

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>

namespace fs = std::filesystem;
using namespace VGG;

class UIApplication : public app::EventListener
{
  app::AppRender* m_layer{ nullptr };
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<Controller> m_controller;

public:
  void setLayer(app::AppRender* layer)
  {
    ASSERT(layer);
    m_layer = layer;
  }

  void setView(std::shared_ptr<UIView> view)
  {
    ASSERT(view);
    m_view = view;

    ASSERT(m_layer);
    m_layer->addAppScene(m_view);
  }

  void setController(std::shared_ptr<Controller> controller)
  {
    ASSERT(controller);
    m_controller = controller;

    ASSERT(m_layer);
    m_layer->addAppRenderable(m_controller->editor());
  }

  bool onEvent(UEvent evt, void* userData) override
  {
    if (evt.type == VGG_APP_INIT)
    {
      return true;
    }

    switch (evt.type)
    {
      case VGG_KEYDOWN:
        break; // break to continue processing

      case VGG_WINDOWEVENT:
      {
        switch (evt.window.event)
        {
          case VGG_WINDOWEVENT_RESIZED:
          case VGG_WINDOWEVENT_SIZE_CHANGED:
            m_view->setDirty(true);
            break;

          default:
            break;
        }
      }
      default:
        return false;
    }

    auto key = evt.key.keysym.sym;
    auto mod = evt.key.keysym.mod;

    if (key == VGGK_PAGEUP && (mod & VGG_KMOD_CTRL))
    {
      INFO("Previous page");
      m_view->preArtboard();
      return true;
    }

    if (key == VGGK_PAGEDOWN && (mod & VGG_KMOD_CTRL))
    {
      INFO("Next page");
      m_view->nextArtboard();
      return true;
    }

    if (key == VGGK_e && (mod & VGG_KMOD_CTRL))
    {
      INFO("Switch edit mode");
      m_controller->setEditMode(!m_controller->isEditMode());
      m_view->setDirty(true);
      return true;
    }

    if (key == VGGK_b)
    {
      INFO("Toggle object bounding box");
      Scene::enableDrawDebugBound(!Scene::isEnableDrawDebugBound());
      m_view->setDirty(true);
      return true;
    }

    if (key == VGGK_1)
    {
      INFO("Toggle cursor position");
      m_layer->setDrawPositionEnabled(!m_layer->enableDrawPosition());
      return true;
    }

    return false;
  }

  void run(int fps)
  {
    if (m_layer->beginFrame(fps))
    {
      if (m_view->isDirty() || m_controller->hasDirtyEditor())
      {
        m_view->setDirty(false);
        m_controller->resetEditorDirty();

        m_layer->render();
        m_layer->endFrame();
      }
    }
  }
};
