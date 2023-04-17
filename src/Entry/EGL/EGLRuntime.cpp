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

const char *get_egl_error_info(EGLint error) {
  switch (error) {
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

#define EGL_CHECK()                                                         \
  do {                                                                         \
    auto error = eglGetError();                                                \
    if (error != EGL_SUCCESS) {                                                \
        WARN("EGL Error: %s", get_egl_error_info(error));                      \
    }                                                                          \
  } while (0)

constexpr int opengl_version[] = {3, 3};
constexpr int pixel_buffer_width = 800;
constexpr int pixel_buffer_height = 600;

constexpr EGLint config_attribs[] = {EGL_SURFACE_TYPE,
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
                                     EGL_NONE};


constexpr EGLint context_attris[] = {EGL_CONTEXT_MAJOR_VERSION,
                                     opengl_version[0],
                                     EGL_CONTEXT_MINOR_VERSION,
                                     opengl_version[1],
                                     EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                     EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                     EGL_NONE};


class EGLRuntime : public App<EGLRuntime>
{
  EGLContext egl_ctx;
  EGLSurface egl_surface;
  EGLDisplay egl_display;
  std::unordered_map<std::string, std::any> m_properties;
public:
#ifdef __linux__
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
  DPI::ScaleFactor = get_scale_factor();
  int winWidth = w * DPI::ScaleFactor;
  int winHeight = h * DPI::ScaleFactor;

  m_properties["window_size"] = std::pair{winWidth, winHeight};
  m_properties["viewport_size"] = std::pair{winWidth, winHeight};
  m_properties["app_size"] = std::pair{winWidth, winHeight};

  egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (egl_display == EGL_NO_DISPLAY) {
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
    EGLConfig egl_config = nullptr;
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
    if(eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_ctx) == EGL_TRUE){
      return true;
    }
    return false;
  }

  std::any getProperty(const std::string& name)
  {
    // the properties result doesnt matter in EGL backend
	auto it = m_properties.find(name);
	if(it != m_properties.end()){
		return it->second;
	}
	return std::any();
  }

  void setProperty(const std::string& name, std::any value)
  {
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
    if(eglSwapBuffers(egl_display, egl_surface) == EGL_FALSE){
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

int init(int width, int height){
  app = App<EGLRuntime>::getInstance(width, height, "VGG");
  if(!app){
	return -1;
  }
  return 0;
}

int shutdown(){
  if(app){
	delete app;
  app = nullptr;
  }
	return 0;
}

int loadDocument(const char * doc){
  return -1;
}

int loadSketchFile(const char * filename){
  if(!FileManager::loadFile(filename)){
      FAIL("Failed to load file: %s", filename);
      return -1;
  }
  return 0;
}


int renderAsImage(int artboardID, const ImageInfo * info, Image * out){
	ASSERT(app);
	if(!app){
    FAIL("App is not initialized");
    return -1;
	}
  app->frame();
	auto canvas = app->getCanvas();
  auto surface = app->getSurface();
  if(surface){
    auto image = surface->makeImageSnapshot();
    if(image){
      auto data = image->encodeToData(SkEncodedImageFormat::kPNG, info->quality);
      if(data){
        out->size = data->size();
        try{
          out->data = new uint8_t[data->size()];
        }catch(...){
          FAIL("allocate memory for image failed");
          return -4;
        }
        memcpy(out->data, data->bytes(), out->size);
        return 0;
      }
      FAIL("null data");
      return -3;
    }
    FAIL("null image");
    return -2;
  }
  FAIL("null surface");
	// render to png
  return -1;
}

void releaseImage(Image * image){
  if(image){
    return;
    image->size = 0;
    delete image->data;
    image->data = nullptr;
  }
}
