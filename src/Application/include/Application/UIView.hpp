#pragma once

#include "AppScene.h"
#include "UIEvent.hpp"
#include "ViewModel.hpp"

#include <Domain/Layout/Node.hpp>
#include <Scene/Scene.h>

#include <nlohmann/json.hpp>

#include <memory>
#include <tuple>
#include <vector>

namespace VGG
{

class UIView : public app::AppScene
{
public:
  using EventListener = std::function<void(UIEventPtr, std::weak_ptr<LayoutNode>)>;
  using ResourcesType = std::map<std::string, std::vector<char>>;
  using HasEventListener = std::function<bool(const std::string&, UIEventType)>;

  using ScalarType = int;

private:
  friend class Editor;

  EventListener m_eventListener;
  HasEventListener m_hasEventListener;

  UIView* m_superview{ nullptr };
  std::vector<std::shared_ptr<UIView>> m_subviews;

  std::weak_ptr<LayoutNode> m_document;
  int m_page{ 0 };

  Layout::Rect m_frame;
  Layout::Rect m_bounds;

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
  {
    setZoomerListener(std::make_shared<app::AppZoomer>());
  }

  void show(const ViewModel& viewModel)
  {
    loadFileContent(viewModel.designDoc);
    m_document = viewModel.layoutTree;
    // todo, merge edited doc resouces ?
    Scene::setResRepo(viewModel.resources());

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

  bool onEvent(UEvent e, void* userData) override;

  void addSubview(std::shared_ptr<UIView> view)
  {
    m_subviews.push_back(view);
    view->m_superview = this;
    layoutSubviews();
  }

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

  void nextArtboard();
  void preArtboard();

private:
  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);

  void layoutSubviews();
  Layout::Point converPointFromWindow(Layout::Point point);
  Layout::Point converPointFromWindowAndScale(Layout::Point point);
  std::shared_ptr<LayoutNode> currentPage();
};

} // namespace VGG
