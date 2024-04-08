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

#include "Adapter/Container/QtGraphicsContext.hpp"

#include "VGG/Container/QtContainer.hpp"

namespace VGG
{

// impl ----------------------------------------------------------------------
class QtContainerImpl : public IContainer
{
  friend QtContainer;

  QtContainer*               m_api;
  std::unique_ptr<Container> m_impl;

public:
  QtContainerImpl(QtContainer* api)
    : m_api(api)
  {
    m_impl.reset(new Container);
  }

  IContainer* container() override
  {
    return m_impl.get();
  }

  void init(int w, int h, float devicePixelRatio)
  {
    auto theContext = new QtGraphicsContext();
    theContext->init(devicePixelRatio);

    std::unique_ptr<VGG::layer::SkiaGraphicsContext> graphicsContext{ theContext };
    m_impl->setGraphicsContext(graphicsContext, w, h);
  }
};

// api ----------------------------------------------------------------------

QtContainer::QtContainer()
  : m_impl(new QtContainerImpl(this))
{
}

QtContainer::~QtContainer() = default;

void QtContainer::init(int w, int h, float devicePixelRatio)
{
  m_impl->init(w, h, devicePixelRatio);
}

IContainer* QtContainer::container()
{
  return m_impl.get();
}

} // namespace VGG