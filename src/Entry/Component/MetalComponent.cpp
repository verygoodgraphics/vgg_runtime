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

#include "Component.hpp"

#include "Adapter/Component/MetalGraphicsContext.h"

#include "VGG/Component/MetalComponent.hpp"

namespace VGG
{

// impl ----------------------------------------------------------------------
class MetalComponentImpl
{
  friend MetalComponent;

  MetalComponent*            m_api;
  std::unique_ptr<Component> m_component;

public:
  MetalComponentImpl(MetalComponent* api)
    : m_api(api)
  {
    m_component.reset(new Component);
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

  void setView(MetalComponent::MTLHandle mtkView)
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

MetalComponent::MetalComponent()
  : m_impl(new MetalComponentImpl(this))
{
}

MetalComponent::~MetalComponent() = default;

bool MetalComponent::load(
  const std::string& filePath,
  const char*        designDocSchemaFilePath,
  const char*        layoutDocSchemaFilePath)
{

  return m_impl->load(filePath, designDocSchemaFilePath, layoutDocSchemaFilePath);
}

bool MetalComponent::run()
{
  return m_impl->run();
}

void MetalComponent::setView(MTLHandle mtkView)
{
  m_impl->setView(mtkView);
}

bool MetalComponent::onEvent(UEvent evt)
{
  return m_impl->onEvent(evt);
}

void MetalComponent::setEventListener(EventListener listener)
{
  return m_impl->m_component->setEventListener(listener);
}

} // namespace VGG