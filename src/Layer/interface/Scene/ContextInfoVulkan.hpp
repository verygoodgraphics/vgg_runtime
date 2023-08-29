#pragma once

#include <vulkan/vulkan_core.h>

struct ContextInfoVulkan
{
  VkInstance instance{ VK_NULL_HANDLE };
  VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
  VkQueue queue{ VK_NULL_HANDLE };
  VkDevice device{ VK_NULL_HANDLE };
  VkSurfaceKHR surface{ VK_NULL_HANDLE };
  int graphicsQueueIndex{ -1 };
  PFN_vkVoidFunction getProc{ nullptr };
};
