#pragma once

#include <include/gpu/vk/GrVkBackendContext.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/third_party/vulkan/vulkan/vulkan_core.h>
#include "Scene/ContextInfoVulkan.hpp"
#include "SkiaImpl/VSkiaContext.hpp"

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

SkiaContext::SurfaceCreateProc vkSurfaceCreateProc()
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

SkiaContext::ContextCreateProc vkContextCreateProc(ContextInfoVulkan* context)
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
