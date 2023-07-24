#pragma once

#include "UIEvent.hpp"
#include "Scene/Scene.h"

#include "nlohmann/json.hpp"

#include <memory>
#include <tuple>
#include <vector>

union SDL_Event;
class SkCanvas;

namespace VGG
{

class UIView
{
  std::shared_ptr<Scene> m_scene;
  std::vector<std::shared_ptr<UIView>> m_subviews;

public:
  using EventListener = std::function<void(UIEventPtr)>;
  using ResourcesType = std::map<std::string, std::vector<char>>;

  UIView()
    : m_scene{ std::make_shared<Scene>() }
  {
  }

  auto scene()
  {
    return m_scene;
  }

  void show(const nlohmann::json& viewModel)
  {
    m_scene->loadFileContent(viewModel);
  }

  void setResouces(ResourcesType resources)
  {
    Scene::ResRepo = std::move(resources);
  }

  void setEventListener(EventListener listener)
  {
    m_event_listener = listener;
  }

  void onEvent(const SDL_Event& evt);

  void addSubview(std::shared_ptr<UIView> view)
  {
    m_subviews.push_back(view);
  }
  void draw(SkCanvas* canvas);

private:
  EventListener m_event_listener;

  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);
};

} // namespace VGG