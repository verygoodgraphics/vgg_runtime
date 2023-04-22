#include <algorithm>
#include <filesystem>
#include <any>
#include <nlohmann/json.hpp>
#include <EGL/egl.h>

#include "../../Entity/EntityManager.hpp"
#include "../../Entity/InputManager.hpp"
#include "../../Systems/RenderSystem.hpp"
#include "../../Utils/Utils.hpp"
#include "../../Utils/Version.hpp"
#include "../../Utils/App.hpp"
#include "../../Utils/FileManager.hpp"

#include "EGLRuntime.h"

using namespace VGG;
using namespace std;

const char* get_egl_error_info(EGLint error)
{
  switch (error)
  {
    case EGL_NOT_INITIALIZED:
      return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:
      return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:
      return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:
      return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONTEXT:
      return "EGL_BAD_CONTEXT";
    case EGL_BAD_CONFIG:
      return "EGL_BAD_CONFIG";
    case EGL_BAD_CURRENT_SURFACE:
      return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:
      return "EGL_BAD_DISPLAY";
    case EGL_BAD_SURFACE:
      return "EGL_BAD_SURFACE";
    case EGL_BAD_MATCH:
      return "EGL_BAD_MATCH";
    case EGL_BAD_PARAMETER:
      return "EGL_BAD_PARAMETER";
    case EGL_BAD_NATIVE_PIXMAP:
      return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:
      return "EGL_BAD_NATIVE_WINDOW";
    case EGL_CONTEXT_LOST:
      return "EGL_CONTEXT_LOST";
    case EGL_SUCCESS:
      return "NO ERROR";
    default:
      return "UNKNOWN ERROR";
  }
}

#define EGL_CHECK()                                                                                \
  do                                                                                               \
  {                                                                                                \
    auto error = eglGetError();                                                                    \
    if (error != EGL_SUCCESS)                                                                      \
    {                                                                                              \
      WARN("EGL Error: %s", get_egl_error_info(error));                                            \
    }                                                                                              \
  } while (0)

constexpr int opengl_version[] = { 3, 3 };
constexpr int pixel_buffer_width = 800;
constexpr int pixel_buffer_height = 600;

constexpr EGLint config_attribs[] = { EGL_SURFACE_TYPE,
                                      EGL_PBUFFER_BIT,
                                      EGL_BLUE_SIZE,
                                      8,
                                      EGL_GREEN_SIZE,
                                      8,
                                      EGL_RED_SIZE,
                                      8,
                                      EGL_DEPTH_SIZE,
                                      8,
                                      EGL_RENDERABLE_TYPE,
                                      EGL_OPENGL_BIT,
                                      EGL_NONE };

constexpr EGLint context_attris[] = { EGL_CONTEXT_MAJOR_VERSION,
                                      opengl_version[0],
                                      EGL_CONTEXT_MINOR_VERSION,
                                      opengl_version[1],
                                      EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                      EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                      EGL_NONE };

class EGLRuntime : public App<EGLRuntime>
{
  EGLContext egl_ctx;
  EGLSurface egl_surface;
  EGLDisplay egl_display;
  EGLConfig egl_config = nullptr;
  std::unordered_map<std::string, std::any> m_properties;

private:
  void resizeEGLSurface(int w, int h)
  {

    EGLint width = w;
    EGLint height = h;

    eglDestroySurface(egl_display, egl_surface);
    EGL_CHECK();
    EGLint pixel_buffer_attribs[] = {
      EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE,
    };
    egl_surface = eglCreatePbufferSurface(egl_display, egl_config, pixel_buffer_attribs);
    EGL_CHECK();

    eglBindAPI(EGL_OPENGL_API);
    // Make the context current with the new surface
    makeContextCurrent();
    EGL_CHECK();
  }

public:
#ifdef VGG_HOST_Linux
  static double get_scale_factor()
  {
    static constexpr int NVARS = 4;
    static const char* vars[NVARS] = {
      "FORCE_SCALE",
      "QT_SCALE_FACTOR",
      "QT_SCREEN_SCALE_FACTOR",
      "GDK_SCALE",
    };
    for (int i = 0; i < NVARS; i++)
    {
      const char* strVal = getenv(vars[i]);
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
  static inline double get_scale_factor()
  {
    return 1.0;
  }
#endif
  bool initContext(int w, int h, const std::string& title)
  {
    // DPI::ScaleFactor = get_scale_factor();
    // int winWidth = w * DPI::ScaleFactor;
    // int winHeight = h * DPI::ScaleFactor;
    int winWidth = w;
    int winHeight = h;

    m_properties["window_size"] = std::pair{ winWidth, winHeight };
    m_properties["viewport_size"] = std::pair{ winWidth, winHeight };
    m_properties["app_size"] = std::pair{ winWidth, winHeight };

    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_display == EGL_NO_DISPLAY)
    {
      WARN("No default display");
      // try EXT_platform_device, see
      // https://www.khronos.org/registry/EGL/extensions/EXT/EGL_EXT_platform_device.txt
      // egl_display = create_display_from_device();
      return false;
    }

    EGLint major = 0, minor = 0;
    eglInitialize(egl_display, &major, &minor);
    INFO("EGL version: %d, %d", major, minor);
    INFO("EGL vendor string: %s", eglQueryString(egl_display, EGL_VENDOR));
    EGL_CHECK();

    // 2. Select an appropriate configuration
    EGLint num_configs = 0;
    eglChooseConfig(egl_display, config_attribs, &egl_config, 1, &num_configs);
    EGL_CHECK();

    // 3. Create a surface

    EGLint pixel_buffer_attribs[] = {
      EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE,
    };
    egl_surface = eglCreatePbufferSurface(egl_display, egl_config, pixel_buffer_attribs);
    EGL_CHECK();

    // 4. Bind the API
    eglBindAPI(EGL_OPENGL_API);
    EGL_CHECK();

    // 5. Create a context and make it current
    egl_ctx = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attris);
    EGL_CHECK();
    return makeContextCurrent();
  }
  bool makeContextCurrent()
  {
    if (eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_ctx) == EGL_TRUE)
    {
      return true;
    }
    return false;
  }

  std::any getProperty(const std::string& name)
  {
    // the properties result doesnt matter in EGL backend
    auto it = m_properties.find(name);
    if (it != m_properties.end())
    {
      return it->second;
    }
    return std::any();
  }

  void setProperty(const std::string& name, std::any value)
  {
    auto it = m_properties.find(name);
    if (it != m_properties.end())
    {
      if (name == "viewport_size")
      {
        auto size = std::any_cast<std::pair<int, int>>(value);
        auto prop = std::any_cast<std::pair<int, int>>(it->second);
        if (size.first != prop.first || size.second != prop.second)
        {
          resizeEGLSurface(prop.first, prop.second);
          updateSkiaEngine();
          resizeSkiaSurface(prop.first, prop.second);
          it->second = value;
        }
      }
    }
  }

  void onInit()
  {
  }

  void pollEvent()
  {
    // no event need to deal with
  }

  void swapBuffer()
  {
    if (eglSwapBuffers(egl_display, egl_surface) == EGL_FALSE)
    {
      WARN("egl swap buffer failed\n");
    }
  }
  ~EGLRuntime()
  {
    eglDestroyContext(egl_display, egl_ctx);
    eglDestroySurface(egl_display, egl_surface);
  }
};

static EGLRuntime* app = nullptr;

SkEncodedImageFormat _toSkFormat(EncodeFormat format)
{
  switch (format)
  {
    case EncodeFormat::PNG:
      return SkEncodedImageFormat::kPNG;
    default:
      WARN("Unkonwn format, fallback to PNG");
  }
  return SkEncodedImageFormat::kPNG;
}

int _renderCurrentPage(int quality, EncodeFormat format, Image* out)
{
  app->frame(0);
  auto canvas = app->getCanvas();
  auto surface = app->getSurface();
  if (surface)
  {
    auto image = surface->makeImageSnapshot();
    if (image)
    {
      auto data = image->encodeToData(_toSkFormat(format), quality);
      if (data)
      {
        out->size = data->size();
        try
        {
          out->data = new uint8_t[out->size];
        }
        catch (...)
        {
          FAIL("bad alloc");
          return -1;
        }
        memcpy(out->data, data->bytes(), out->size);
        return 0;
      }
      return -2;
    }
    return -2;
  }
  return -2;
}

int init(int width, int height)
{
  app = App<EGLRuntime>::getInstance(width, height, "VGG");
  // app->setProperty("viewport_size", std::pair<int,int>{width, height});
  if (!app)
  {
    return -1;
    FAIL("Initialize failed\n");
  }
  return 0;
}

void shutdown()
{
  // do nothing
}

int loadSketchFile(const char* filename)
{
  if (!FileManager::loadFile(filename))
  {
    FAIL("Failed to load file: %s", filename);
    return -1;
  }
  return 0;
}

int loadContent(const char* content)
{
  if (!app)
  {
    FAIL("not init");
    return -1;
  }
  app->getScene()->LoadFileContent(std::string(content));
  return 0;
}

int renderAsImages(int width, int height, const ImageInfo* infos, int count, Image* out)
{
  ASSERT(app);
  if (!app)
  {
    FAIL("App is not initialized");
    return -1;
  }

  auto fm = FileManager::getInstance();
  if (!fm->currFile)
  {
    FAIL("No File loaded");
    return -1;
  }

  // update image size
  auto size = std::pair<int, int>{ width, height };
  app->setProperty("viewport_size", size);

  int res = 0;
  for (int i = 0; i < count; i++)
  {
    auto currentFile = fm->currFile;
    currentFile->currPageIdx = infos[i].artboardID;
    currentFile->loadCurrPage();
    out[i].data = nullptr;
    if (_renderCurrentPage(infos[i].quality, infos[i].format, out + i) < 0)
    {
      res++;
    }
  }
  return res == 0 ? 0 : -2;
}

void releaseImage(Image* image, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (image[i].data)
    {
      delete image->data;
      image->size = 0;
      image->data = nullptr;
    }
  }
}
