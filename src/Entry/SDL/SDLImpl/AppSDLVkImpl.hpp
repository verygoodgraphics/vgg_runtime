#pragma once

#include <Entry/SDL/SDLImpl/EventAPISDLImpl.h>
#include "Event/EventAPI.h"
#include <Application/interface/AppBase.hpp>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include <Utility/interface/Log.h>
#include "EventConvert.h"
#include "nlohmann/json.hpp"
#include <memory>
#include <optional>
#include <any>
#include <Scene/GraphicsContext.h>
#include <Scene/ContextInfoVulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "VulkanObject.hpp"

namespace VGG::entry
{

using namespace VGG::app;
using namespace VGG::layer;
class AppSDLImpl : public AppBase<AppSDLImpl>
{
  struct SDLState
  {
    SDL_Window* window{ nullptr };
    std::shared_ptr<vk::VkInstanceObject> vkInstance;
    std::shared_ptr<vk::VkPhysicalDeviceObject> vkPhysicalDevice;
    std::shared_ptr<vk::VkDeviceObject> vkDevice;
    std::shared_ptr<vk::VkSurfaceObject> vkSurface;
    ContextInfoVulkan context;
  };
  SDLState m_sdlState;
  using Getter = std::function<std::any(void)>;
  using Setter = std::function<void(std::any)>;
  std::unordered_map<std::string, std::pair<Getter, Setter>> m_properties;

public:
  static inline void handleVkError()
  {
  }

#ifdef __linux__
  static double getScaleFactor()
  {
    static constexpr int NVARS = 4;
    static const char* s_vars[NVARS] = {
      "FORCE_SCALE",
      "QT_SCALE_FACTOR",
      "QT_SCREEN_SCALE_FACTOR",
      "GDK_SCALE",
    };
    for (int i = 0; i < NVARS; i++)
    {
      const char* strVal = getenv(s_vars[i]);
      if (strVal)
      {
        double val = atof(strVal);
        if (val >= 1.0)
        {
          return val;
        }
      }
    }
    return 1.0;
  }
#else
  static inline double getScaleFactor()
  {
    return 1.0;
  }
#endif

  void onInitProperties(layer::ContextProperty& property) override
  {
    property.resolutionScale = resolutionScale();
  }

  bool onInit() override
  {
    const auto& cfg = config();
    if (initContext(cfg.windowSize[0], cfg.windowSize[1]))
      return false;

    // Reigster SDL event impl
    auto eventAPIImpl = std::make_unique<EventAPISDLImpl>();
    EventManager::registerEventAPI(std::move(eventAPIImpl));
    return true;
  }

  void shutdown() override
  {
    if (m_sdlState.window)
    {
      SDL_DestroyWindow(m_sdlState.window);
    }
    SDL_Quit();
  }

  void* contextInfo() override
  {
    return &m_sdlState.context;
  }

  std::optional<AppError> initContext(int w, int h)
  {
    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      handleVkError();
      return AppError(AppError::EKind::RenderEngineError, "sdl init failed");
    }
    SDL_version compileVersion, linkVersion;
    SDL_VERSION(&compileVersion);
    SDL_GetVersion(&linkVersion);
    INFO("SDL version %d.%d.%d (linked %d.%d.%d)",
         compileVersion.major,
         compileVersion.minor,
         compileVersion.patch,
         linkVersion.major,
         linkVersion.minor,
         linkVersion.patch);
    //
    // create window
    float f = getScaleFactor();
    int winWidth = w * f;
    int winHeight = h * f;
    SDL_Window* window =
      SDL_CreateWindow(appName().c_str(),
                       SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED,
                       winWidth,
                       winHeight,
                       SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window)
    {
      handleVkError();
      return AppError(AppError::EKind::RenderEngineError, "Create Window Failed\n");
    }
    m_sdlState.window = window;

    m_sdlState.vkInstance = std::make_shared<vk::VkInstanceObject>();
    m_sdlState.vkPhysicalDevice =
      std::make_shared<vk::VkPhysicalDeviceObject>(m_sdlState.vkInstance);
    m_sdlState.vkDevice = std::make_shared<vk::VkDeviceObject>(m_sdlState.vkPhysicalDevice);
    m_sdlState.context.instance = *(m_sdlState.vkInstance);
    m_sdlState.context.physicalDevice = *(m_sdlState.vkPhysicalDevice);
    m_sdlState.context.device = *(m_sdlState.vkDevice);
    m_sdlState.context.graphicsQueueIndex = m_sdlState.vkDevice->graphicsQueueIndex;

    if (SDL_Vulkan_CreateSurface(m_sdlState.window,
                                 m_sdlState.context.instance,
                                 &m_sdlState.context.surface) == SDL_FALSE)
    {
      DEBUG("Create Vulkan surface failed\n");
    }

    m_sdlState.vkSurface = std::make_shared<vk::VkSurfaceObject>(m_sdlState.vkInstance,
                                                                 m_sdlState.vkDevice,
                                                                 m_sdlState.context.surface);

    return std::nullopt;
  }

public:
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
    // In SDL, we do not need to resize its surface mannually
    return true;
  }

  std::optional<AppError> makeContextCurrent()
  {
    return std::nullopt;
  }

  float resolutionScale()
  {
#ifdef VGG_HOST_macOS
    int dw, dh;
    int ww, wh;
    SDL_Vulkan_GetDrawableSize(this->m_sdlState.window, &dw, &dh);
    SDL_GetWindowSize(this->m_sdlState.window, &ww, &wh);
    const float s = float(dw) / (float)ww;
    DEBUG("Scale Factor on macOS: %f", s);
    return s;
#else
    return getScaleFactor();
#endif
  }

  void pollEvent()
  {
    SDL_Event evt;
    while (SDL_PollEvent(&evt))
    {
      sendEvent(toUEvent(evt));
    }
  }

  void swapBuffer()
  {
    return;
  }

  ~AppSDLImpl()
  {
  }
};
} // namespace VGG::entry
