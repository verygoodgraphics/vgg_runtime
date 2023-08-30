#pragma once
#include "VulkanObject.hpp"
#include <vulkan/vulkan_core.h>

class VulkanContext
{

public:
  enum class EContextConfig
  {
    WINDOWED_CONTEXT,
    WINDOWLESS_CONTEXT
  };
  // TODO: refactor as unique_ptr
  std::shared_ptr<vk::VkInstanceObject> vkInstance;
  std::shared_ptr<vk::VkPhysicalDeviceObject> vkPhysicalDevice;
  std::shared_ptr<vk::VkDeviceObject> vkDevice;

  // windowlesss-specific
  std::shared_ptr<vk::VkSurfaceObject> vkSurface;
  std::shared_ptr<vk::VkSwapchainObject> vkSwapchain;
  VkQueue presentQueue{ VK_NULL_HANDLE };
  uint32_t swapChainImageIndex = -1;
  VkFence fence;
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;

  EContextConfig config;
  VulkanContext(const VulkanContext&) = delete;
  VulkanContext& operator=(const VulkanContext&) = delete;

  VulkanContext(VulkanContext&&) noexcept = delete;
  VulkanContext& operator=(VulkanContext&&) noexcept = delete;

  VulkanContext(EContextConfig cfg)
    : config(std::move(cfg))
  {
    if (cfg == EContextConfig::WINDOWED_CONTEXT)
    {
      createWindowedContext();
    }
    else if (cfg == EContextConfig::WINDOWLESS_CONTEXT)
    {
      createWindowlessContext();
    }
  }

  void createWindowedContext()
  {
    cleanup();
  }

  void createWindowlessContext()
  {
    cleanup();
    vkInstance = std::make_shared<vk::VkInstanceObject>(
      [this]()
      {
        std::vector<const char*> extNames;
        return extNames;
      });
    vkPhysicalDevice = std::make_shared<vk::VkPhysicalDeviceObject>(vkInstance);
    vkDevice = std::make_shared<vk::VkDeviceObject>(vkPhysicalDevice);
  }

  VkDevice device()
  {
    ASSERT(vkDevice);
    return *vkDevice;
  }

  VkQueue queue()
  {
    ASSERT(vkDevice);
    return vkDevice->graphicsQueue;
  }

  int queueIndex()
  {
    ASSERT(vkDevice);
    return vkDevice->graphicsQueueIndex;
  }

  VkInstance instance()
  {
    ASSERT(vkInstance);
    return *vkInstance;
  }

  VkPhysicalDevice physicalDevice()
  {
    ASSERT(vkPhysicalDevice);
    return *vkPhysicalDevice;
  }

  VkQueue preQueue()
  {
    ASSERT(presentQueue != VK_NULL_HANDLE);
    ASSERT(config == EContextConfig::WINDOWED_CONTEXT);
    return presentQueue;
  }

  ~VulkanContext()
  {
    cleanup();
  }

  void resetFence()
  {
    ASSERT(vkDevice);
    vkResetFences(*vkDevice, 1, &fence);
  }

  void cleanup()
  {

    if (vkSurface)
    {
      vkSurface = nullptr;
    }
    if (vkSwapchain)
    {
      vkSwapchain = nullptr;
    }
    if (vkDevice)
    {
      vkDevice = nullptr;
    }
    if (vkPhysicalDevice)
    {
      vkPhysicalDevice = nullptr;
    }
    if (vkInstance)
    {
      vkInstance = nullptr;
    }
    if (vkDevice != VK_NULL_HANDLE && imageAvailableSemaphore != VK_NULL_HANDLE)
    {
      vkDestroySemaphore(*vkDevice, imageAvailableSemaphore, nullptr);
    }
    if (vkDevice != VK_NULL_HANDLE && renderFinishedSemaphore != VK_NULL_HANDLE)
    {
      vkDestroySemaphore(*vkDevice, renderFinishedSemaphore, nullptr);
    }
    if (vkDevice != VK_NULL_HANDLE && fence != VK_NULL_HANDLE)
    {
      vkDestroyFence(*vkDevice, fence, nullptr);
    }
  }

  void acquireImageIndex()
  {
    ASSERT(vkDevice);
    vkAcquireNextImageKHR(*vkDevice,
                          vkSwapchain->swapChainKHR,
                          UINT64_MAX,
                          imageAvailableSemaphore,
                          VK_NULL_HANDLE,
                          &swapChainImageIndex);
  }

  void createSemaphore()
  {
    ASSERT(vkDevice);
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(*vkDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) !=
          VK_SUCCESS ||
        vkCreateSemaphore(*vkDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) !=
          VK_SUCCESS ||
        vkCreateFence(*vkDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create semaphores!");
    }
  }

  bool submit()
  {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = &commandBuffer;
    //
    if (vkQueueSubmit(presentQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
      WARN("submit failed");
      return false;
    }
    return true;
  }

  void present()
  {
    VkPresentInfoKHR presentInfo;
    VkSwapchainKHR swapChains[] = { vkSwapchain->swapChainKHR };
    presentInfo.pSwapchains = swapChains;
    presentInfo.swapchainCount = 1;
    presentInfo.pResults = 0;
    vkQueuePresentKHR(presentQueue, &presentInfo);
  }
};
