#include <Application/interface/AppBase.hpp>
#include <EGL/egl.h>
#include <Core/FontManager.h>
#include <Core/PaintNode.h>
#include <fstream>
#include <Application/interface/Event/EventListener.h>
#include <Scene/GraphicsContext.h>
#include <Scene/GraphicsLayer.h>
#include <Scene/VGGLayer.h>
#include <algorithm>
#include <exception>
#include <memory>
#include <optional>
#include <Domain/Layout/ExpandSymbol.hpp>
using namespace std;
using namespace VGG::app;
namespace VGG::entry
{

constexpr int OPENGL_VERSION[] = { 4, 5 };
constexpr int PIXEL_BUFFER_WIDTH = 800;
constexpr int PIXEL_BUFFER_HEIGHT = 600;
constexpr EGLint CONFIG_ATTRIBS[] = { EGL_SURFACE_TYPE,
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

constexpr EGLint CONTEXT_ATTRIBS[] = { EGL_CONTEXT_MAJOR_VERSION,
                                       OPENGL_VERSION[0],
                                       EGL_CONTEXT_MINOR_VERSION,
                                       OPENGL_VERSION[1],
                                       EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                       EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                       EGL_NONE };

const char* getEglErrorInfo(EGLint error)
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
      WARN("EGL Error: %s", getEglErrorInfo(error));                                               \
    }                                                                                              \
  } while (0)

class EGLCtx : public layer::GraphicsContext
{
  EGLContext m_eglCtx;
  EGLSurface m_eglSurface;
  EGLDisplay m_eglDisplay;
  EGLConfig m_eglConfig = nullptr;
  std::unordered_map<std::string, std::any> m_properties;

private:
  void resizeEGLSurface(int w, int h)
  {

    EGLint width = w;
    EGLint height = h;

    eglDestroySurface(m_eglDisplay, m_eglSurface);
    EGL_CHECK();
    EGLint pixelBufferAttribs[] = {
      EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE,
    };
    m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, m_eglConfig, pixelBufferAttribs);
    EGL_CHECK();

    eglBindAPI(EGL_OPENGL_API);
    // Make the context current with the new surface
    makeContextCurrent();
    EGL_CHECK();
  }

public:
  float getDPIScale()
  {
    return 1.0;
  }
  std::optional<AppError> initContext(int w, int h)
  {
    int winWidth = w;
    int winHeight = h;

    m_properties["window_size"] = std::pair{ winWidth, winHeight };
    m_properties["viewport_size"] = std::pair{ winWidth, winHeight };
    m_properties["app_size"] = std::pair{ winWidth, winHeight };

    m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_eglDisplay == EGL_NO_DISPLAY)
    {
      WARN("No default display");
      // try EXT_platform_device, see
      // https://www.khronos.org/registry/EGL/extensions/EXT/EGL_EXT_platform_device.txt
      // egl_display = create_display_from_device();
      return AppError(AppError::EKind::EGLNoDisplayError, "No default display");
    }

    EGLint major = 0, minor = 0;
    eglInitialize(m_eglDisplay, &major, &minor);
    INFO("EGL version: %d, %d", major, minor);
    INFO("EGL vendor string: %s", eglQueryString(m_eglDisplay, EGL_VENDOR));
    EGL_CHECK();

    // 2. Select an appropriate configuration
    EGLint numConfigs = 0;
    eglChooseConfig(m_eglDisplay, CONFIG_ATTRIBS, &m_eglConfig, 1, &numConfigs);
    EGL_CHECK();

    int maxSurfaceSize[2];
    if (eglGetConfigAttrib(m_eglDisplay, m_eglConfig, EGL_MAX_PBUFFER_WIDTH, maxSurfaceSize) !=
        EGL_TRUE)
    {
      EGL_CHECK();
      return AppError(AppError::EKind::EGLGetAttribError, "get attribute error");
    }
    if (eglGetConfigAttrib(m_eglDisplay, m_eglConfig, EGL_MAX_PBUFFER_HEIGHT, maxSurfaceSize + 1) !=
        EGL_TRUE)
    {
      EGL_CHECK();
      return AppError(AppError::EKind::EGLGetAttribError, "get attribute error");
    }

    if (winWidth + 100 > maxSurfaceSize[0] || winHeight + 100 > maxSurfaceSize[1])
    {
      return AppError(AppError::EKind::TextureSizeOutOfRangeError, "Texture size out of range");
    }

    // 3. Create a surface

    EGLint pixelBufferAttribs[] = {
      EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE,
    };
    m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, m_eglConfig, pixelBufferAttribs);
    EGL_CHECK();

    // 4. Bind the API
    eglBindAPI(EGL_OPENGL_API);
    EGL_CHECK();

    // 5. Create a context and make it current
    m_eglCtx = eglCreateContext(m_eglDisplay, m_eglConfig, EGL_NO_CONTEXT, CONTEXT_ATTRIBS);
    EGL_CHECK();

    return makeContextCurrent();
  }
  std::optional<AppError> makeContextCurrent()
  {
    if (eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglCtx) == EGL_TRUE)
    {
      return nullopt;
    }
    EGL_CHECK();
    return AppError(AppError::EKind::MakeCurrentContextError, "make current failed");
  }

  bool swap() override
  {
    swapBuffer();
    return true;
  }

  bool makeCurrent() override
  {
    return true;
  }

  void* contextInfo() override
  {
    return m_eglCtx;
  }

  bool onResize(int w, int h) override
  {
    return true;
  }

  virtual void shutdown() override
  {
    destoryEGLContext();
  }

  void onInitProperties(layer::ContextProperty& property) override
  {
    // No property need to be init in EGL
  }

  bool onInit() override
  {
    const auto& cfg = config();
    if (initContext(cfg.windowSize[0], cfg.windowSize[1]))
      return false;
    return true;
  }

  void pollEvent()
  {
    // no event need to deal with
  }

  void swapBuffer()
  {
    if (eglSwapBuffers(m_eglDisplay, m_eglSurface) == EGL_FALSE)
    {
      WARN("egl swap buffer failed\n");
    }
  }

  void destoryEGLContext()
  {
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_eglDisplay, m_eglCtx);
    eglDestroySurface(m_eglDisplay, m_eglSurface);
    eglTerminate(m_eglDisplay);
    // eglReleaseThread();
  }

  ~EGLCtx()
  {
    destoryEGLContext();
  }
};

void getMaxSurfaceSize(int resolutionLevel, float* maxSurfaceSize)
{
  switch (resolutionLevel)
  {
    case 0:
    {
      maxSurfaceSize[0] = 2048;
      maxSurfaceSize[1] = 2048;
    }
    break;
    case 1:

    {
      maxSurfaceSize[0] = 2048;
      maxSurfaceSize[1] = 2048;
    }
    break;
    case 2:
    {
      maxSurfaceSize[0] = 4096;
      maxSurfaceSize[1] = 4096;
    }
    break;
    case 3:
    {
      maxSurfaceSize[0] = 8192;
      maxSurfaceSize[1] = 8192;
    }
    break;
    case 4:
    {
      maxSurfaceSize[0] = 16384;
      maxSurfaceSize[1] = 16384;
    }
    break;
    default:
    {
      maxSurfaceSize[0] = 2048;
      maxSurfaceSize[1] = 2048;
    }
  }
}

float calcScaleFactor(float inputWidth,
                      float inputHeight,
                      float maxWidth,
                      float maxHeight,
                      float& outWidth,
                      float& outHeight)
{
  auto widthScale = maxWidth / inputWidth;
  auto heightScale = maxHeight / inputHeight;
  float outputSize[2] = { 0.f, 0.f };
  if (widthScale < heightScale)
  {
    outWidth = maxWidth;
    outHeight = widthScale * inputHeight;
  }
  else
  {
    outWidth = heightScale * inputWidth;
    outHeight = maxHeight;
  }
  return widthScale > heightScale ? heightScale : widthScale;
}
std::tuple<std::string, std::vector<std::pair<std::string, std::vector<char>>>> render(
  const nlohmann::json& j,
  const std::map<std::string, std::vector<char>>& resources,
  int imageQuality,
  int resolutionLevel,
  const std::string& configFile,
  const std::string& fontCollectionName)
{

  Config::readGlobalConfig(configFile);
  float maxSurfaceSize[2];
  getMaxSurfaceSize(resolutionLevel, maxSurfaceSize);
  std::stringstream ss;

  auto egl = std::make_shared<EGLCtx>();
  auto layer = std::make_shared<layer::VLayer>();
  layer::ContextConfig cfg;
  cfg.windowSize[0] = maxSurfaceSize[0];
  cfg.windowSize[1] = maxSurfaceSize[1];
  cfg.stencilBit = 8;
  cfg.multiSample = 0;
  egl->init(cfg);
  layer->init(egl.get());

  FontManager::instance().setDefaultFontManager(fontCollectionName);
  std::vector<std::pair<std::string, std::vector<char>>> res;
  auto scene = std::make_shared<Scene>();
  // expand symbol
  Layout::ExpandSymbol e(j);
  scene->loadFileContent(e());
  scene->setResRepo(resources);
  layer->addScene(scene);
  auto count = scene->frameCount();
  for (int i = 0; i < count; i++)
  {
    auto b = scene->frame(i)->getBound();
    int w = b.size().x;
    int h = b.size().y;
    scene->setPage(i);

    float actualSize[2];
    auto scale =
      calcScaleFactor(w, h, maxSurfaceSize[0], maxSurfaceSize[1], actualSize[0], actualSize[1]);
    layer->setScaleFactor(scale);

    DEBUG("Render Canvas Size: [%d, %d]", (int)maxSurfaceSize[0], (int)maxSurfaceSize[1]);
    DEBUG("Original [%d, %d] x Scale[%f] = Final[%d, %d]",
          w,
          h,
          scale,
          (int)actualSize[0],
          (int)actualSize[1]);

    // begin render one frame
    layer->beginFrame();
    layer->render();
    layer->endFrame();
    // end render one frame

    layer::ImageOptions opts;
    opts.encode = layer::EImageEncode::IE_PNG;
    opts.position[0] = 0;
    opts.position[1] = 0;
    opts.extend[0] = actualSize[0];
    opts.extend[1] = actualSize[1];
    opts.quality = imageQuality;
    if (auto img = layer->makeImageSnapshot(opts); img)
    {
      static int s_count = 0;
      std::ofstream ofs(scene->frame(i)->guid() + ".png", std::ios::binary);
      if (ofs.is_open())
      {
        ofs.write((const char*)img->data(), img->size());
      }
      res.emplace_back(scene->frame(i)->guid(), std::move(img.value()));
    }
    else
    {
      ss << "Failed to encode image for artboard: " << i + 1 << std::endl;
    }
  }
  return { std::string{ std::istreambuf_iterator<char>{ ss }, std::istreambuf_iterator<char>{} },
           res };
}

} // namespace VGG::entry
