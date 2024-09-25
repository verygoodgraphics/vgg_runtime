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

#include "Container.hpp"

#include "Adapter/Container/AndroidGraphicsContext.h"

#include "VGG/Container/AndroidContainer.hpp"

namespace VGG
{

// impl ----------------------------------------------------------------------
class AndroidContainerImpl : public IContainer
{
  friend AndroidContainer;

  AndroidContainer*          m_api;
  std::unique_ptr<Container> m_impl;

public:
  AndroidContainerImpl(AndroidContainer* api)
    : m_api(api)
  {
    m_impl.reset(new Container);
  }

  IContainer* container() override
  {
    return m_impl.get();
  }

  void setView(ANativeWindow* window)
  {
    INFO("AndroidContainerImpl::setView, window: %p", window);
    if (window)
    {
      auto androidContext = new AndroidGraphicsContext(window);
      std::unique_ptr<VGG::layer::SkiaGraphicsContext> graphicsContext{ androidContext };
      m_impl->setGraphicsContext(
        graphicsContext,
        androidContext->width(),
        androidContext->height());
      __android_log_print(ANDROID_LOG_DEBUG, "VGG", "setGraphicsContext");
    }
    else
    {
      // TODO: remove graphics context, eg active window is paused, lose GL context
    }
  }
};

// api ----------------------------------------------------------------------

AndroidContainer::AndroidContainer()
  : m_impl(new AndroidContainerImpl(this))
{
}

AndroidContainer::~AndroidContainer() = default;

void AndroidContainer::setView(ANativeWindow* window)
{
  m_impl->setView(window);
}

IContainer* AndroidContainer::container()
{
  return m_impl.get();
}

} // namespace VGG
