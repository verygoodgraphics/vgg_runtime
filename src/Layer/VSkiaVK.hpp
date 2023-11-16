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
#include "VSkiaContext.hpp"

#include "Layer/Graphics/ContextInfoVulkan.hpp"

#include <include/gpu/vk/GrVkBackendContext.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/third_party/vulkan/vulkan/vulkan_core.h>

#define VK_PROC_BEGIN(vkName)                                                                      \
  if (strcmp(name, #vkName) == 0)                                                                  \
  {                                                                                                \
    ptr = reinterpret_cast<PFN_vkVoidFunction>(vkName);                                            \
  }
#define VK_PROC(vkName)                                                                            \
  else if (strcmp(name, #vkName) == 0)                                                             \
  {                                                                                                \
    ptr = reinterpret_cast<PFN_vkVoidFunction>(vkName);                                            \
  }

#define VK_PROC_END                                                                                \
  if (ptr)                                                                                         \
    return ptr;
namespace VGG::layer::skia_impl::vk
{

SurfaceCreateProc vkSurfaceCreateProc()
{
  return [](GrDirectContext* context, int w, int h, const ContextConfig& cfg)
  {
    ASSERT(context);
    GrVkImageInfo vkImageInfo;
    vkImageInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    GrBackendRenderTarget target(w, h, vkImageInfo);
    SkImageInfo info = SkImageInfo::Make(w,
                                         h,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    return SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
  };
}

ContextCreateProc vkContextCreateProc(ContextInfoVulkan* context)
{
  return [context]()
  {
    ASSERT(context->instance);
    ASSERT(context->physicalDevice);
    ASSERT(context->device);
    ASSERT(context->queue);
    GrVkBackendContext vkContext;
    vkContext.fInstance = context->instance;
    vkContext.fPhysicalDevice = context->physicalDevice;
    vkContext.fDevice = context->device;
    vkContext.fQueue = context->queue;
    vkContext.fGetProc = std::function(
      [=](const char* name, VkInstance instance, VkDevice dev) -> PFN_vkVoidFunction
      {
        PFN_vkVoidFunction ptr = nullptr;
        if (instance == VK_NULL_HANDLE && dev == VK_NULL_HANDLE)
        {
          ptr = vkGetInstanceProcAddr(VK_NULL_HANDLE, name);
        }
        else if (instance == VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
        {
          ptr = vkGetDeviceProcAddr(dev, name);
        }
        else if (instance != VK_NULL_HANDLE && dev == VK_NULL_HANDLE)
        {
          ptr = vkGetInstanceProcAddr(instance, name);
        }
        else
        {
          ptr = vkGetDeviceProcAddr(dev, name);
        }
        if (!ptr)
          DEBUG("xtension [%s] required by skia is not satisfied", name);
        return ptr;
      });
    auto grContext = GrDirectContext::MakeVulkan(vkContext);
    return grContext;
  };
}
} // namespace VGG::layer::skia_impl::vk
