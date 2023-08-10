#pragma once

#include "Domain/Layout/View.hpp"
#include "UIEvent.hpp"
#include "Scene/Scene.h"
#include "ViewModel.hpp"

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
  UIView* m_superview{ nullptr };
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

  bool m_is_dirty = false;

public:
  UIView()
    : m_scene{ std::make_shared<Scene>() }
  {
  }

  auto scene()
  {
    return m_scene;
  }

  void show(const ViewModel& viewModel)
  {
    m_scene->loadFileContent(viewModel.designDoc);
    m_root = viewModel.layoutTree;
    // todo, merge edited doc resouces ?
    Scene::getResRepo() = std::move(viewModel.resources());

    m_is_dirty = true;
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

  Layout::Size size()
  {
    return m_frame.size;
  }
  void setSize(scalar_type w, scalar_type h)
  {
    m_frame.size.width = w;
    m_frame.size.height = h;

    m_is_dirty = true;

    layoutSubviews();
  }

  void becomeEditorWithSidebar(scalar_type top,
                               scalar_type right,
                               scalar_type bottom,
                               scalar_type left);

  bool isDirty()
  {
    return m_is_dirty;
  }
  void setDirty(bool dirty)
  {
    m_is_dirty = dirty;
  }

private:
  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);

  void layoutSubviews();
  Layout::Point converPointFromWindow(Layout::Point point);
  Layout::Point converPointFromWindowAndScale(Layout::Point point);
  void handleMouseWheel(const SDL_Event& evt, Zoomer* zoomer);
};

} // namespace VGG
