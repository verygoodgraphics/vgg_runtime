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
#include "UIApplication.hpp"
#include <algorithm>
#include <iterator>
#include <optional>
#include "AppRender.hpp"
#include "Controller.hpp"
#include "Domain/Layout/Rect.hpp"
#include "Event/Event.hpp"
#include "Event/Keycode.hpp"
#include "Layer/Core/EventManager.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"
#include "Application/VGGLayer.hpp"
#include "UIScrollView.hpp"

namespace VGG
{

void UIApplication::setView(std::shared_ptr<UIScrollView> view, double w, double h)
{
  ASSERT(view);
  if (m_view == view)
  {
    m_view->setSize(w, h);
    return;
  }

  m_view = view;
  m_view->setSize(w, h);

  ASSERT(m_layer);
  m_layer->addEventListener(m_view);
  m_view->setLayer(m_layer);
}

bool UIApplication::onEvent(UEvent evt, void* userData)
{
  if (evt.type == VGG_APP_INIT)
  {
    return true;
  }

  switch (evt.type)
  {
    case VGG_KEYDOWN:
      return handleKeyEvent(evt.key);

    case VGG_WINDOWEVENT:
    {
      switch (evt.window.event)
      {
        case VGG_WINDOWEVENT_RESIZED:
        case VGG_WINDOWEVENT_SIZE_CHANGED:
          m_view->setSize(evt.window.data1, evt.window.data2);
          m_controller->onResize();
          break;

        default:
          break;
      }
    }

    case VGG_MOUSEWHEEL:
      return m_controller->handleMouseWheel(evt.wheel.preciseX, evt.wheel.preciseY);

    case VGG_TOUCHDOWN:
    case VGG_TOUCHMOTION:
    case VGG_TOUCHUP:
      return m_controller->handleTouchEvent(evt.touch);

    default:
      return false;
  }
}

bool UIApplication::paint(int fps, bool force)
{
  m_view->updateOncePerLoop();

  if (force || needsPaint())
  {
    if (force)
    {
      fps = 0;
    }

    m_controller->updateDisplayContentIfNeeded();
    if (m_layer->beginFrame(fps))
    {
      m_layer->render();
      m_layer->endFrame();

      if (m_firstRender)
      {
        m_firstRender = false;
        m_controller->onFirstRender();
      }

      m_view->frame();

      m_controller->postFrame();
      return true;
    }
  }

  return false;
}

bool UIApplication::needsPaint()
{
  return m_view->isDirty() || m_controller->hasDirtyEditor() || layer::EventManager::hasEvents();
}

bool UIApplication::handleKeyEvent(VKeyboardEvent evt)
{
  auto key = evt.keysym.sym;
  auto mod = evt.keysym.mod;

  if (key == VGGK_PAGEUP && (mod & VGG_KMOD_CTRL))
  {
    m_controller->previouseFrame();
    return true;
  }

  if (key == VGGK_PAGEDOWN && (mod & VGG_KMOD_CTRL))
  {
    m_controller->nextFrame();
    return true;
  }

  if (key == VGGK_e && (mod & VGG_KMOD_CTRL))
  {
    INFO("Switch edit mode");
    m_controller->setEditMode(!m_controller->isEditMode());
    m_view->setDirty(true);
    m_view->setDrawBackground(m_controller->isEditMode());
    return true;
  }

  if (key == VGGK_b)
  {
    INFO("Toggle object bounding box");
    m_layer->setDebugModeEnabled(!m_layer->debugModeEnabled());
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

void UIApplication::setLayer(app::AppRender* layer)
{
  ASSERT(layer);
  m_layer = layer;

#ifdef EMSCRIPTEN
  m_layer->setSamplingOptions(SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest));
#endif
}

std::vector<uint8_t> UIApplication::makeImageSnapshot(layer::ImageOptions options)
{
  std::vector<uint8_t> result;
  ASSERT(m_layer);
  options.position[0] = 0;
  options.position[1] = 0;
  const auto& viewSize = m_view->size();
  const auto  dpi = m_layer->context()->property().dpiScaling;
  options.extend[0] = viewSize.width * dpi;
  options.extend[1] = viewSize.height * dpi;

  // const auto oldScaleFactor = m_layer->scaleFactor();
  // m_layer->setScaleFactor(oldScaleFactor / dpi);
  if (auto maybeImageBytes = m_layer->makeImageSnapshot(options))
  {
    const auto& bytes = maybeImageBytes.value();
    std::copy(bytes.begin(), bytes.end(), std::back_inserter(result));
  }

  // m_layer->setScaleFactor(oldScaleFactor);

  return result;
}

} // namespace VGG
