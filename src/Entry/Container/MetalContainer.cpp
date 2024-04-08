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

#include "Adapter/Container/MetalGraphicsContext.h"

#include "VGG/Container/MetalContainer.hpp"

namespace VGG
{

// impl ----------------------------------------------------------------------
class MetalContainerImpl : public IContainer
{
  friend MetalContainer;

  MetalContainer*            m_api;
  std::unique_ptr<Container> m_impl;

public:
  MetalContainerImpl(MetalContainer* api)
    : m_api(api)
  {
    m_impl.reset(new Container);
  }

  IContainer* container() override
  {
    return m_impl.get();
  }

  void setView(MetalContainer::MTLHandle mtkView)
  {
    auto metalContext = new MetalGraphicsContext(mtkView);
    std::unique_ptr<VGG::layer::SkiaGraphicsContext> graphicsContext{ metalContext };
    m_impl->setGraphicsContext(graphicsContext, metalContext->width(), metalContext->height());
  }
};

// api ----------------------------------------------------------------------

MetalContainer::MetalContainer()
  : m_impl(new MetalContainerImpl(this))
{
}

MetalContainer::~MetalContainer() = default;

void MetalContainer::setView(MTLHandle mtkView)
{
  m_impl->setView(mtkView);
}

IContainer* MetalContainer::container()
{
  return m_impl.get();
}

} // namespace VGG