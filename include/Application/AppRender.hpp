#pragma once
#include "Event/Event.hpp"
#include "AppScene.hpp"
#include "AppRenderable.hpp"

#include "VGG/Layer/VGGLayer.hpp"

#include <queue>

namespace VGG::app
{

class AppRender__pImpl;
class AppRender : public layer::VLayer
{
  VGG_DECL_IMPL(AppRender)

public:
  AppRender();
  void postEvent(UEvent e, void* userData);
  void sendEvent(UEvent e, void* userData);
  void addAppRenderable(std::shared_ptr<AppRenderable> listener);
  void addAppScene(std::shared_ptr<AppScene> listener);
  std::shared_ptr<AppScene> popAppScene();
  bool beginFrame(int fps);
  void render();
  void endFrame();

  ~AppRender();

protected:
  bool onEvent(UEvent e);
};

} // namespace VGG::app
