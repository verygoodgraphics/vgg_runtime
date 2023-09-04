#pragma once

#include "Domain/Layout/Node.hpp"
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

  using ScalarType = int;

private:
  EventListener m_eventListener;
  HasEventListener m_hasEventListener;

  std::shared_ptr<Scene> m_scene;
  UIView* m_superview{ nullptr };
  std::vector<std::shared_ptr<UIView>> m_subviews;

  std::shared_ptr<LayoutNode> m_root;

  Layout::Rect m_frame;
  Layout::Rect m_bounds;
  float m_contentScaleFactor{ 1.0 };

  // editor
  bool m_isEditor = false;
  // sidebar
  ScalarType m_top{ 0 };
  ScalarType m_right{ 0 };
  ScalarType m_bottom{ 0 };
  ScalarType m_left{ 0 };

  bool m_isDirty = false;

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

    m_isDirty = true;
  }

  void registerEventListener(HasEventListener hasEventListener)
  {
    m_hasEventListener = hasEventListener;
  }

  void setEventListener(EventListener listener)
  {
    m_eventListener = listener;
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
  void setSize(ScalarType w, ScalarType h)
  {
    m_frame.size.width = w;
    m_frame.size.height = h;

    m_isDirty = true;

    layoutSubviews();
  }

  void becomeEditorWithSidebar(ScalarType top,
                               ScalarType right,
                               ScalarType bottom,
                               ScalarType left);

  bool isDirty()
  {
    return m_isDirty;
  }
  void setDirty(bool dirty)
  {
    m_isDirty = dirty;
  }

private:
  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);

  void layoutSubviews();
  Layout::Point converPointFromWindow(Layout::Point point);
  Layout::Point converPointFromWindowAndScale(Layout::Point point);
  void handleMouseWheel(const SDL_Event& evt, Zoomer* zoomer);
};

} // namespace VGG
