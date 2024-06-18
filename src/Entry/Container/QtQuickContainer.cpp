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

#include "VGG/Container/QtQuickContainer.hpp"
#include "Adapter/Container/QtQuickGraphicsContext.hpp"
#include "Container.hpp"

namespace VGG
{

class QtQuickContainerImpl : public IContainer
{
  friend QtQuickContainer;

  QtQuickContainer*          m_api;
  std::unique_ptr<Container> m_impl;
  QtQuickGraphicsContext*    m_qtQuickGraphicscontext;

public:
  QtQuickContainerImpl(QtQuickContainer* api)
    : m_api(api)
    , m_qtQuickGraphicscontext{ nullptr }
  {
    m_impl.reset(new Container);
  }

  IContainer* container() override
  {
    return m_impl.get();
  }

  void setFboID(unsigned int fboID)
  {
    assert(m_qtQuickGraphicscontext);
    m_qtQuickGraphicscontext->setFboID(fboID);
  }

  void init(
    int          width,
    int          height,
    double       devicePixelRatio,
    unsigned int fboID,
    int          stencilBit,
    int          multiSample)
  {
    std::unique_ptr<VGG::layer::SkiaGraphicsContext> graphicsContext{
      new QtQuickGraphicsContext(fboID, devicePixelRatio)
    };

    m_qtQuickGraphicscontext = dynamic_cast<QtQuickGraphicsContext*>(graphicsContext.get());
    m_impl->setGraphicsContext(graphicsContext, width, height, stencilBit, multiSample);
  }
};

QtQuickContainer::QtQuickContainer(
  int          width,
  int          height,
  double       devicePixelRatio,
  unsigned int fboID,
  int          stencilBit,
  int          multiSample)
  : m_impl(new QtQuickContainerImpl(this))
{
  m_impl->init(width, height, devicePixelRatio, fboID, stencilBit, multiSample);
}

QtQuickContainer ::~QtQuickContainer()
{
}

IContainer* QtQuickContainer::container()
{
  return m_impl.get();
}

void QtQuickContainer::setFboID(unsigned int fboID)
{
  m_impl->setFboID(fboID);
}

} // namespace VGG
