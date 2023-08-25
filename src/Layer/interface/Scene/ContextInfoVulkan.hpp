#pragma once

#include <vulkan/vulkan_core.h>

struct ContextInfoVulkan
{
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkQueue queue;
  VkDevice device;
  VkSurfaceKHR surface;
  int graphicsQueueIndex{ -1 };
  PFN_vkVoidFunction getProc;
};
