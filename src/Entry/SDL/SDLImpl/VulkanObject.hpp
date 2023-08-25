#pragma once
#include <Utility/interface/Log.h>
#include <cstdint>
#include <private/base/SkTDArray.h>
#include <vector>
#include <iostream>
#include <functional>
#include <memory>

#include <GLFW/glfw3.h>

#include <vulkan/vulkan_core.h>

#ifndef __APPLE__
inline const std::vector<const char*> g_validationLayers = {
  "VK_LAYER_LUNARG_standard_validation"
};
#else
const std::vector<const char*> g_validationLayers = {};
#endif
inline const std::vector<const char*> g_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#define VK_CHECK(expr)                                                                             \
  do                                                                                               \
  {                                                                                                \
    VkResult res;                                                                                  \
    if ((res = expr) != VK_SUCCESS)                                                                \
    {                                                                                              \
      std::cout << "VkResult(" << res << "): " << #expr << ": Vulkan Assertion Failed:";           \
      exit(-1);                                                                                    \
    }                                                                                              \
  } while (false);

constexpr bool ENABLE_VALIDATION_LAYER = true;

namespace vk
{
struct VkInstanceObject : public std::enable_shared_from_this<VkInstanceObject>
{
  VkInstanceObject()
  {
    uint32_t extPropCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extPropCount, nullptr);
    std::vector<VkExtensionProperties> props(extPropCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extPropCount, props.data());
    for (int i = 0; i < props.size(); i++)
    {
      std::cout << props[i].extensionName << std::endl;
    }
    if (glfwInit() != GLFW_TRUE)
    {
      std::cout << "glfw init failed\n";
      exit(-1);
    };
    uint32_t extCount = 0;
    auto ext = glfwGetRequiredInstanceExtensions(&extCount);
    std::vector<const char*> extNames(ext, ext + extCount);
    for (const auto& c : extNames)
    {
      std::cout << "Extension: " << c << std::endl;
    }

    if (ENABLE_VALIDATION_LAYER)
    {
      extNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkInstanceCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pNext = nullptr;
    ci.enabledLayerCount = 0;
    ci.ppEnabledLayerNames = nullptr;

    ci.enabledExtensionCount = extNames.size();
    ci.ppEnabledExtensionNames = extNames.data();

    ci.pApplicationInfo = nullptr;
    ////Check validationLayers
    if (false)
    { // warning: If layer is enabled, the VK_LAYER_PATH must be set
      ci.enabledLayerCount = g_validationLayers.size();
      ci.ppEnabledLayerNames = g_validationLayers.data();
    }

    VK_CHECK(vkCreateInstance(&ci, allocationCallback, &m_vkInstance));
#ifndef NDEBUG
    if (ENABLE_VALIDATION_LAYER)
    {
      VkDebugUtilsMessengerCreateInfoEXT ci = {};
      ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      ci.pNext = nullptr;
      ci.pUserData = nullptr;
      ci.pfnUserCallback = debugCallback;
      ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

      m_debugUtilsMessenger = createDebugUtilsMessenger(ci);
      if (m_debugUtilsMessenger == VK_NULL_HANDLE)
      {
        std::cout << "Create Debug Utils messenger failed\n";
        exit(-1);
      }
    }
#endif
  }
  ~VkInstanceObject()
  {
    destroyDebugUtilsMessenger(m_debugUtilsMessenger);
    vkDestroyInstance(m_vkInstance, allocationCallback);
  }
  operator VkInstance()
  {
    return m_vkInstance;
  }
  VkAllocationCallbacks* allocationCallback = nullptr;

private:
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData)
  {
    (void)messageSeverity;
    (void)messageType;
    (void)pUserData;
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
  }
  VkDebugUtilsMessengerEXT createDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT& ci)
  {
    auto func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance,
                                                                "vkCreateDebugUtilsMessengerEXT");
    VkDebugUtilsMessengerEXT vkHandle = VK_NULL_HANDLE;
    if (func)
    {
      VK_CHECK(func(m_vkInstance, &ci, allocationCallback, &vkHandle));
    }
    else
    {
      std::cout << "Failed to load vkCreateDebugUtilsMessengerEXT\n";
      exit(-1);
    }
    return vkHandle;
  }
  void destroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT object)
  {
    auto func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance,
                                                                 "vkDestroyDebugUtilsMessengerEXT");
    if (func)
    {
      func(m_vkInstance, object, allocationCallback);
    }
    else
    {
      std::cout << "Failed to load vkDestoryDebugUtilsMessengerEXT\n";
    }
  }

  VkInstance m_vkInstance;
  VkDebugUtilsMessengerEXT m_debugUtilsMessenger = VK_NULL_HANDLE;
};

struct VkPhysicalDeviceObject : public std::enable_shared_from_this<VkPhysicalDeviceObject>
{
  std::shared_ptr<VkInstanceObject> instance;
  std::vector<VkMemoryType> allowedMemoryType;
  std::vector<VkQueueFamilyProperties> queueFamilies;
  VkPhysicalDeviceFeatures physicalDeviceFeatures;

  VkPhysicalDeviceObject() = default;
  VkPhysicalDeviceObject(std::shared_ptr<VkInstanceObject> instance)
    : instance(std::move(instance))
  {
    uint32_t count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(*(this->instance), &count, nullptr));
    using namespace std;
    vector<VkPhysicalDevice> dev(count);
    VK_CHECK(vkEnumeratePhysicalDevices(*(this->instance), &count, dev.data()));
    std::cout << "There are " << count << " physical device(s)\n";

    for (int i = 0; i < count; i++)
    {
      VkPhysicalDeviceProperties prop;
      auto d = dev[i];
      vkGetPhysicalDeviceProperties(d, &prop);
      VkPhysicalDeviceMemoryProperties memProp;
      vkGetPhysicalDeviceMemoryProperties(d, &memProp);
      for (int i = 0; i < memProp.memoryTypeCount; i++)
      {
        allowedMemoryType.push_back(memProp.memoryTypes[i]);
      }

      // vkGetPhysicalDeviceProperties(d,&prop);
      // vkGetPhysicalDeviceFeatures(d, &feat);
      //  We don't do any assumption for device, just choose the first physical device
      m_physicalDevice = d;

      // Get queue families
      uint32_t count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
      assert(count > 0);
      queueFamilies.resize(count);
      vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, queueFamilies.data());

      // Get PhysicalDeviceFeatures
      vkGetPhysicalDeviceFeatures(m_physicalDevice, &physicalDeviceFeatures);

      break;
    }
  }

  VkPhysicalDeviceObject(const VkPhysicalDeviceObject&) = delete;
  VkPhysicalDeviceObject& operator=(const VkPhysicalDeviceObject&) = delete;
  VkPhysicalDeviceObject(VkPhysicalDeviceObject&&) noexcept = default;
  VkPhysicalDeviceObject& operator=(VkPhysicalDeviceObject&&) noexcept = default;

  operator VkPhysicalDevice()
  {
    return m_physicalDevice;
  }
  uint32_t findProperMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags)
  {
    for (int i = 0; i < allowedMemoryType.size(); i++)
    {
      if ((typeFilter & (1 << i)) && (allowedMemoryType[i].propertyFlags & flags) == flags)
      {
        return allowedMemoryType[i].heapIndex;
      }
    }
    ASSERT(false && "Failed to find proper memory type");
    return ~0;
  }

  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features)
  {
    for (VkFormat format : candidates)
    {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);
      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
      {
        return format;
      }
      else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features)
      {
        return format;
      }
    }
    std::cout << "Unsupported format\n";
    exit(-1);
  }
  VkSampleCountFlagBits getMaxUsableSampleCount()
  {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);

    VkSampleCountFlags counts = std::min(props.limits.framebufferColorSampleCounts,
                                         props.limits.framebufferDepthSampleCounts);
    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
      return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
      return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
      return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
      return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
      return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
      return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
  }

  const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures()
  {
    return physicalDeviceFeatures;
  }

  int getQueueIndex(VkQueueFlagBits queueFlags) const
  {
    const auto count = queueFamilies.size();
    for (int i = 0; i < count; i++)
    {
      const auto q = queueFamilies[i];
      if (q.queueCount > 0 && q.queueFlags & queueFlags)
      {
        return i;
      }
    }
    std::cout << "specified queue is not found\n";
    return -1;
  }

  VkBool32 surfaceSupport() const
  {
    VkBool32 support = VK_FALSE;
    // vkGetPhysicalDeviceSurfaceSupportKHR( *(PhysicalDevice ), GraphicsQueueIndex, Surface,
    // &support );
    return support;
  }

  VkFormat findDepthFormat()
  {
    return findSupportedFormat(
      { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  bool hasDepthStencil(VkFormat format)
  {
    return (format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT);
  }

private:
  VkPhysicalDevice m_physicalDevice;
};

struct VkDeviceObject : public std::enable_shared_from_this<VkDeviceObject>
{
  VkDeviceObject() = default;
  // std::unordered_map<uint32_t,std::shared_ptr<VkDeviceMemoryObject>> Memory;
  VkDeviceObject(std::shared_ptr<VkPhysicalDeviceObject> physicalDevice)
    : physicalDevice(std::move(physicalDevice))
  {
    VkDeviceCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.pNext = nullptr;

    graphicsQueueIndex = this->physicalDevice->getQueueIndex(VK_QUEUE_GRAPHICS_BIT);

    VkDeviceQueueCreateInfo queCI = {};
    queCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queCI.pNext = nullptr;
    queCI.queueFamilyIndex = graphicsQueueIndex;
    queCI.queueCount = 1;
    float pri = 1.0;
    queCI.pQueuePriorities = &pri;

    VkPhysicalDeviceFeatures feat = {};
    feat = this->physicalDevice->getPhysicalDeviceFeatures();
    feat.samplerAnisotropy = VK_TRUE;
    feat.sampleRateShading = VK_TRUE;

    ci.pQueueCreateInfos = &queCI;
    ci.queueCreateInfoCount = 1;

    ci.enabledLayerCount = g_validationLayers.size();
    ci.ppEnabledLayerNames = g_validationLayers.data();

    ci.ppEnabledExtensionNames = g_deviceExtensions.data(); // for swap chain
    ci.enabledExtensionCount = g_deviceExtensions.size();

    ci.pEnabledFeatures = &feat;

    VK_CHECK(vkCreateDevice(*this->physicalDevice,
                            &ci,
                            this->physicalDevice->instance->allocationCallback,
                            &m_device));

    vkGetDeviceQueue(m_device, graphicsQueueIndex, 0, &graphicsQueue);

    using namespace std;

    vector<VkDescriptorPoolSize> poolSize{ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
                                           { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10 },
                                           { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 } };

    VkDescriptorPoolCreateInfo dpCI = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr,        0, 1,
      static_cast<uint32_t>(poolSize.size()),        poolSize.data()
    };
    // DescriptorPool = CreateDescriptorPool(dpCI);
  }
  ~VkDeviceObject()
  {
    vkDestroyDevice(m_device, physicalDevice->instance->allocationCallback);
  }

  VkDeviceObject(const VkDeviceObject&) = delete;
  VkDeviceObject& operator=(const VkDeviceObject&) = delete;

  VkDeviceObject(VkDeviceObject&& rhs) noexcept
    : physicalDevice(std::move(rhs.physicalDevice))
    , m_device(rhs.m_device)
  {
    rhs.m_device = VK_NULL_HANDLE;
  }

  VkDeviceObject& operator=(VkDeviceObject&& rhs) noexcept
  {
    vkDestroyDevice(m_device, physicalDevice->instance->allocationCallback);
    physicalDevice = std::move(rhs.physicalDevice);
    m_device = rhs.m_device;
    rhs.m_device = VK_NULL_HANDLE;
    return *this;
  }
  operator VkDevice()
  {
    return m_device;
  }
  operator VkDevice() const
  {
    return m_device;
  }
  friend class VkSurfaceObject;
  std::shared_ptr<VkPhysicalDeviceObject> physicalDevice;
  uint32_t graphicsQueueIndex = -1;
  VkQueue graphicsQueue = VK_NULL_HANDLE;

  // VkDescriptorPoolObject descriptorPool;

private:
  VkDevice m_device;
};

struct VkSurfaceObject
{

  VkSurfaceCapabilitiesKHR cap;
  template<typename F>
  VkSurfaceObject(std::shared_ptr<VkInstanceObject> instance,
                  std::shared_ptr<VkDeviceObject> device,
                  VkSurfaceKHR surface)
    : m_instance(std::move(instance))
    , m_device(std::move(device))
    , m_surface(std::move(surface))
  {
    ASSERT(m_surface != VK_NULL_HANDLE);
    VkBool32 support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(*(m_device->physicalDevice),
                                         m_device->graphicsQueueIndex,
                                         m_surface,
                                         &support);
    if (support == VK_FALSE)
    {
      std::cout << "This device does not support present feature\n";
      exit(-1);
    }

    vkGetDeviceQueue(*m_device, m_device->graphicsQueueIndex, 0, &m_presentQueue);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*(m_device->physicalDevice), m_surface, &cap);

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*(m_device->physicalDevice), m_surface, &count, nullptr);
    m_supportedFormat.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(*(m_device->physicalDevice),
                                         m_surface,
                                         &count,
                                         m_supportedFormat.data());

    std::cout << "Supported Format: " << count << "\n";
    for (int i = 0; i < count; i++)
    {
      if (m_supportedFormat[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
          m_supportedFormat[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
        std::cout << "Format Support";
      }
      std::cout << m_supportedFormat[i].format << " " << m_supportedFormat[i].colorSpace
                << std::endl;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(*(m_device->physicalDevice),
                                              m_surface,
                                              &count,
                                              nullptr);
    m_supportedPresentMode.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(*(m_device->physicalDevice),
                                              m_surface,
                                              &count,
                                              m_supportedPresentMode.data());
  }
  ~VkSurfaceObject()
  {
    release();
  }
  VkSurfaceObject(const VkSurfaceObject&) = delete;
  VkSurfaceObject& operator=(const VkSurfaceObject&) = delete;
  VkSurfaceObject(VkSurfaceObject&& rhs) noexcept
    : m_instance(std::move(rhs.m_instance))
    , m_surface(rhs.m_surface)
    , m_device(std::move(rhs.m_device))
  {
    rhs.m_surface = VK_NULL_HANDLE;
  }
  VkSurfaceObject& operator=(VkSurfaceObject&& rhs) noexcept
  {
    release();
    m_instance = std::move(rhs.m_instance);
    m_device = std::move(rhs.m_device);
    return *this;
  }

  operator VkSurfaceKHR()
  {
    return m_surface;
  }
  operator VkSurfaceKHR() const
  {
    return m_surface;
  }

private:
  friend class VkSwapchainObject;
  void release()
  {
    vkDestroySurfaceKHR(*m_instance, m_surface, m_instance->allocationCallback);
    m_device = nullptr;
    m_instance = nullptr;
  }

  VkSurfaceKHR m_surface;
  VkQueue m_presentQueue = VK_NULL_HANDLE;
  std::shared_ptr<VkInstanceObject> m_instance;
  std::shared_ptr<VkDeviceObject> m_device;
  std::vector<VkSurfaceFormatKHR> m_supportedFormat;
  std::vector<VkPresentModeKHR> m_supportedPresentMode;

public:
};
struct VkSwapchainObject
{
  std::vector<VkImage> swapchainImages;
  // std::vector<VkImageViewObject> swapchainImageViews;
  std::shared_ptr<VkSurfaceObject> surface;
  VkSurfaceCapabilitiesKHR cap;
  VkSurfaceFormatKHR surfaceFormat;
  VkExtent2D extent;
  std::shared_ptr<VkDeviceObject> device;

  VkSwapchainKHR swapChainKHR;
  // static FramebufferResizeEventCallback Callback;

  VkSwapchainObject(std::shared_ptr<VkDeviceObject> device,
                    std::shared_ptr<VkSurfaceObject> surface,
                    VkSwapchainKHR oldSwapchain)
    : device(std::move(device))
    , surface(std::move(surface))
  {
    // // Get properties of surface, necessary for creation of swap-chain
    // VkSurfaceCapabilitiesKHR surfaceProperties;
    // if (!getSurfaceProperties(physicalDevice, surface, surfaceProperties))
    //   return false;
    //
    // // Get the image presentation mode (synced, immediate etc.)
    // VkPresentModeKHR presentation_mode = gPresentationMode;
    // if (!getPresentationMode(surface, physicalDevice, presentation_mode))
    //   return false;

    // Get other swap chain related features
    const auto capabilities = surface->cap;
    unsigned int swapImageCount = surface->cap.maxImageCount + 1;

    // Size of the images
    // Default size = window size
    int gWindowWidth, gWindowHeight;
    VkExtent2D swapImageExtent = { (unsigned int)gWindowWidth, (unsigned int)gWindowHeight };

    // This happens when the window scales based on the size of an image
    if (surface->cap.currentExtent.width == 0xFFFFFFF)
    {
      swapImageCount.width = glm::clamp<unsigned int>(size.width,
                                                      capabilities.minImageExtent.width,
                                                      capabilities.maxImageExtent.width);
      swapImageCount.height = glm::clamp<unsigned int>(size.height,
                                                       capabilities.minImageExtent.height,
                                                       capabilities.maxImageExtent.height);
    }
    else
    {
      swapImageExtent = capabilities.currentExtent;
    }

    // Get image usage (color etc.)
    // VkImageUsageFlags usageFlags;
    // if (!getImageUsage(capabilities, usageFlags))
    //   return false;

    // Get the transform, falls back on current transform when transform is not supported
    // VkSurfaceTransformFlagBitsKHR transform = getTransform(capabilities);

    // Get swapchain image format
    // VkSurfaceFormatKHR imageFormat;
    // if (!getFormat(physicalDevice, surface, imageFormat))
    //   return false;

    // Old swap chain

    // Populate swapchain creation info
    VkSwapchainCreateInfoKHR swapInfo;
    swapInfo.pNext = nullptr;
    swapInfo.flags = 0;
    swapInfo.surface = this->surface->m_surface;
    swapInfo.minImageCount = swapImageCount;
    swapInfo.imageFormat = imageFormat.format;
    swapInfo.imageColorSpace = imageFormat.colorSpace;
    swapInfo.imageExtent = swapImageExtent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = usageFlags;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.queueFamilyIndexCount = 0;
    swapInfo.pQueueFamilyIndices = nullptr;
    swapInfo.preTransform = transform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = presentation_mode;
    swapInfo.clipped = true;
    swapInfo.oldSwapchain = NULL;
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    // Destroy old swap chain
    if (oldSwapchain != VK_NULL_HANDLE)
    {
      vkDestroySwapchainKHR(this->device, oldSwapchain, nullptr);
      oldSwapchain = VK_NULL_HANDLE;
    }

    if (vkCreateSwapchainKHR(this->device, &swapInfo, nullptr, &oldSwapchain) != VK_SUCCESS)
    {
      std::cout << "unable to create swap chain\n";
      return false;
    }

    // Store handle
    outSwapChain = oldSwapchain;
  }
  void update()
  {
    *this = VkSwapchainObject(device, surface);
  }
};

}; // namespace vk
