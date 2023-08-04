#pragma once

#include "Domain/Layout/View.hpp"
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
  using EventListener = std::function<void(UIEventPtr)>;
  using ResourcesType = std::map<std::string, std::vector<char>>;
  using HasEventListener = std::function<bool(const std::string&, UIEventType)>;

  using scalar_type = int;

private:
  EventListener m_event_listener;
  HasEventListener m_has_event_listener;

  std::shared_ptr<Scene> m_scene;
  UIView* m_superview;
  std::vector<std::shared_ptr<UIView>> m_subviews;

  std::shared_ptr<LayoutView> m_root;

  Layout::Rect m_frame;
  Layout::Rect m_bounds;
  float m_contentScaleFactor{ 1.0 };

  // editor
  bool m_is_editor = false;
  // sidebar
  scalar_type m_top{ 0 };
  scalar_type m_right{ 0 };
  scalar_type m_bottom{ 0 };
  scalar_type m_left{ 0 };

public:
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
    setupTree(viewModel);
  }

  void setResouces(ResourcesType resources)
  {
    Scene::ResRepo = std::move(resources);
  }

  void registerEventListener(HasEventListener hasEventListener)
  {
    m_has_event_listener = hasEventListener;
  }

  void setEventListener(EventListener listener)
  {
    m_event_listener = listener;
  }

  void onEvent(const SDL_Event& evt, Zoomer* zoomer);

  void addSubview(std::shared_ptr<UIView> view)
  {
    m_subviews.push_back(view);
    view->m_superview = this;
    layoutSubviews();
  }

  void draw(SkCanvas* canvas, Zoomer* zoomer);

  void setSize(scalar_type w, scalar_type h)
  {
    m_frame.size.width = w;
    m_frame.size.height = h;
    layoutSubviews();
  }

  void becomeEditorWithSidebar(scalar_type top,
                               scalar_type right,
                               scalar_type bottom,
                               scalar_type left);

private:
  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);

  void setupTree(const nlohmann::json& j);
  std::shared_ptr<LayoutView> createOneLayoutView(const nlohmann::json& j,
                                                  nlohmann::json::json_pointer current_path,
                                                  std::shared_ptr<LayoutView> parent);
  void createLayoutViews(const nlohmann::json& j,
                         nlohmann::json::json_pointer current_path,
                         std::shared_ptr<LayoutView> parent);

  void createOneOrMoreLayoutViews(const nlohmann::json& j,
                                  nlohmann::json::json_pointer current_path,
                                  std::shared_ptr<LayoutView> parent);
  void layoutSubviews();
  Layout::Point converPointFromWindow(Layout::Point point);
  Layout::Point converPointFromWindowAndScale(Layout::Point point);
  void handleMouseWheel(const SDL_Event& evt, Zoomer* zoomer);
};

} // namespace VGG