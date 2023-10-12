/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "UIView.hpp"

void UIApplication::setView(std::shared_ptr<UIView> view)
{
  ASSERT(view);
  m_view = view;

  ASSERT(m_layer);
  m_layer->addAppScene(m_view);
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
      break; // break to continue processing

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
    m_view->enableDrawDebugBound(!m_view->isEnableDrawDebugBound());
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

void UIApplication::run(int fps)
{
  m_view->updateOncePerLoop();

  if (m_layer->beginFrame(fps))
  {
    if (m_view->isDirty() || m_controller->hasDirtyEditor())
    {
      m_view->setDirty(false);
      m_controller->resetEditorDirty();

      m_layer->render();
      m_layer->endFrame();

      if (m_firstRender)
      {
        m_firstRender = false;
        m_controller->onFirstRender();
      }
    }
  }
}
