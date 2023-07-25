#pragma once

#include "UIEvent.hpp"
#include "Scene/Scene.h"

#include "nlohmann/json.hpp"

#include <memory>
#include <tuple>
#include <vector>

class SkCanvas;
struct Zoomer;
union SDL_Event;

namespace VGG
{

class UIView
{
public:
  using scalar_type = int;

private:
  std::shared_ptr<Scene> m_scene;
  std::vector<std::shared_ptr<UIView>> m_subviews;
  bool m_self_zoom_enabled = true;

  scalar_type m_width{ 0 };
  scalar_type m_height{ 0 };

  // sidebar
  scalar_type m_top{ 0 };
  scalar_type m_right{ 0 };
  scalar_type m_bottom{ 0 };
  scalar_type m_left{ 0 };

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

  void draw(SkCanvas* canvas, Zoomer* zoomer);

  void setSize(scalar_type w, scalar_type h)
  {
    m_width = w;
    m_height = h;
  }
  void becomeEditorWithSidebar(scalar_type top,
                               scalar_type right,
                               scalar_type bottom,
                               scalar_type left);

private:
  EventListener m_event_listener;

  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);
};

} // namespace VGG