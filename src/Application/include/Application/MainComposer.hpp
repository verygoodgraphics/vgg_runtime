#pragma once

#include "Controller.hpp"
#include "PlatformComposer.hpp"
#include "Presenter.hpp"
#include "RunLoop.hpp"
#include "UIView.hpp"
#include "Async.hpp"
#include "Domain/Daruma.hpp"

#include <Application/Mouse.hpp>

#include <memory>

namespace VGG
{

class Editor;

class MainComposer
{
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<UIView> m_edit_view;
  std::shared_ptr<Presenter> m_presenter;
  std::shared_ptr<Editor> m_editor;

  std::shared_ptr<RunLoop> m_runLoop;
  std::shared_ptr<Controller> m_controller;

  std::shared_ptr<PlatformComposer> m_platform_composer;

public:
  MainComposer(PlatformComposer* platformComposer, std::shared_ptr<Mouse> mouse);

  ~MainComposer()
  {
    m_platform_composer->teardown();
  }

  auto runLoop()
  {
    return m_runLoop;
  }

  auto controller()
  {
    return m_controller;
  }

  auto view()
  {
    return m_view;
  }

  void enableEdit(int top = 0, int right = 300, int bottom = 0, int left = 300)
  {
    m_edit_view.reset(new UIView);
    m_presenter->setEditView(m_edit_view);

    if (m_view)
    {
      m_view->becomeEditorWithSidebar(top, right, bottom, left);
      m_view->addSubview(m_edit_view);
    }
  }
};

} // namespace VGG
