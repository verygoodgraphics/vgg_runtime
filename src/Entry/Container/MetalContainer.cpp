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

#include "Container.hpp"

#include "Adapter/Container/MetalGraphicsContext.h"

#include "VGG/Container/MetalContainer.hpp"

namespace VGG
{

// impl ----------------------------------------------------------------------
class MetalContainerImpl
{
  friend MetalContainer;

  MetalContainer*            m_api;
  std::unique_ptr<Container> m_component;

public:
  MetalContainerImpl(MetalContainer* api)
    : m_api(api)
  {
    m_component.reset(new Container);
  }

  bool load(
    const std::string& filePath,
    const char*        designDocSchemaFilePath = nullptr,
    const char*        layoutDocSchemaFilePath = nullptr)
  {
    return m_component->load(filePath, designDocSchemaFilePath, layoutDocSchemaFilePath);
  }

  bool run()
  {
    return m_component->run();
  }

  void setView(MetalContainer::MTLHandle mtkView)
  {
    auto metalContext = new MetalGraphicsContext(mtkView);
    std::unique_ptr<VGG::layer::SkiaGraphicsContext> graphicsContext{ metalContext };
    m_component->setGraphicsContext(graphicsContext, metalContext->width(), metalContext->height());
  }

  bool onEvent(UEvent evt)
  {
    return m_component->onEvent(evt);
  }
};

// api ----------------------------------------------------------------------

MetalContainer::MetalContainer()
  : m_impl(new MetalContainerImpl(this))
{
}

MetalContainer::~MetalContainer() = default;

bool MetalContainer::load(
  const std::string& filePath,
  const char*        designDocSchemaFilePath,
  const char*        layoutDocSchemaFilePath)
{

  return m_impl->load(filePath, designDocSchemaFilePath, layoutDocSchemaFilePath);
}

bool MetalContainer::run()
{
  return m_impl->run();
}

void MetalContainer::setView(MTLHandle mtkView)
{
  m_impl->setView(mtkView);
}

bool MetalContainer::onEvent(UEvent evt)
{
  return m_impl->onEvent(evt);
}

void MetalContainer::setEventListener(EventListener listener)
{
  return m_impl->m_component->setEventListener(listener);
}

} // namespace VGG