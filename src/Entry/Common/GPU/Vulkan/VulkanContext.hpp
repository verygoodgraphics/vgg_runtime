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

  // windowed-specific
  std::shared_ptr<vk::VkSurfaceObject> vkSurface;
  std::shared_ptr<vk::VkSwapchainObject> vkSwapchain;
  VkQueue presentQueue{ VK_NULL_HANDLE };
  uint32_t swapChainImageIndex = -1;
  VkFence fence{ VK_NULL_HANDLE };
  VkSemaphore imageAvailableSemaphore{ VK_NULL_HANDLE };
  VkSemaphore renderFinishedSemaphore{ VK_NULL_HANDLE };

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
    // TODO::
  }

  void createWindowlessContext()
  {
    cleanup();
    vkInstance = std::make_shared<vk::VkInstanceObject>(
      [this]()
      {
        std::vector<const char*> extNames;
        // extNames.push_back("VK_KHR_get_physical_device_properties2");
        // extNames.push_back("VK_KHR_surface");
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
    if (vkDevice)
    {
      if (imageAvailableSemaphore != VK_NULL_HANDLE)
      {
        vkDestroySemaphore(*vkDevice, imageAvailableSemaphore, nullptr);
        imageAvailableSemaphore = VK_NULL_HANDLE;
      }
      if (renderFinishedSemaphore != VK_NULL_HANDLE)
      {
        vkDestroySemaphore(*vkDevice, renderFinishedSemaphore, nullptr);
        renderFinishedSemaphore = VK_NULL_HANDLE;
      }
      if (fence != VK_NULL_HANDLE)
      {
        vkDestroyFence(*vkDevice, fence, nullptr);
        fence = VK_NULL_HANDLE;
      }
    }
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
