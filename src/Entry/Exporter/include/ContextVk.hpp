#pragma once

#include <Scene/GraphicsContext.h>
#include <Entry/Common/GPU/Vulkan/VulkanContext.hpp>

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
    m_context = nullptr;
  }

public:
  VkGraphicsContext()
  {
  }
  void onInitProperties(layer::ContextProperty& property) override
  {
    property.resolutionScale = 1.0;
    property.api = layer::EGraphicsAPIBackend::API_VULKAN;
  }

  bool onInit() override
  {
    return initVulkanContextInfo();
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

  bool onResize(int w, int h) override
  {
    return true;
  }

  ~VkGraphicsContext()
  {
    cleanup();
  }
}; // namespace VGG::entry
} // namespace VGG::exporter
