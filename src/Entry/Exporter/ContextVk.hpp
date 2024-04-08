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
#pragma once
#include "../Common/GPU/Vulkan/VulkanContext.hpp"

#include "Layer/Graphics/GraphicsContext.hpp"
#include "Layer/Graphics/ContextInfoVulkan.hpp"

namespace VGG::exporter
{

using namespace VGG;

class VkGraphicsContext : public layer::GraphicsContext
{
  std::unique_ptr<VulkanContext> m_context;
  ContextInfoVulkan m_vulkanContextInfo;
  bool initVulkanContextInfo()
  {
    m_context = std::make_unique<VulkanContext>(VulkanContext::EContextConfig::WINDOWLESS_CONTEXT);
    ASSERT(m_context);
    if (!m_context)
      return false;
    m_vulkanContextInfo.instance = m_context->instance();
    m_vulkanContextInfo.physicalDevice = m_context->physicalDevice();
    m_vulkanContextInfo.device = m_context->device();
    m_vulkanContextInfo.queue = m_context->queue();
    m_vulkanContextInfo.graphicsQueueIndex = m_context->queueIndex();
    return true;
  }

private:
  void cleanup()
  {
    DEBUG("Vulkan Context releasing...");
    m_context = nullptr;
  }

public:
  VkGraphicsContext()
  {
  }
  void onInitProperties(layer::ContextProperty& property) override
  {
    property.dpiScaling = 1.0;
    property.api = layer::EGraphicsAPIBackend::API_VULKAN;
  }

  bool onInit() override
  {
    return initVulkanContextInfo();
  }

  std::string vulkanInfo()
  {
    return m_context->vkPhysicalDevice->deviceInfo.value_or("");
  }

  void shutdown() override
  {
    cleanup();
  }

  void* contextInfo() override
  {
    if (m_context)
      return &m_vulkanContextInfo;
    return nullptr;
  }

  bool makeCurrent() override
  {
    return true;
  }

  bool swap() override
  {
    return true;
  }

  ~VkGraphicsContext()
  {
    cleanup();
  }
}; // namespace VGG::entry
} // namespace VGG::exporter
