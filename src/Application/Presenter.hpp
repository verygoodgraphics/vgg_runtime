/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "Application/UIEvent.hpp"
#include "Application/UIScrollView.hpp"
#include "Domain/Daruma.hpp"
#include "Domain/ModelEvent.hpp"
#include "Utility/Log.hpp"
#include "ViewModel.hpp"

#include <nlohmann/json.hpp>
#include <rxcpp/rx.hpp>

#include <memory>

namespace VGG
{
class Mouse;
class StateTree;

struct FontInfo
{
  std::string familyName;
  std::string subfamilyName;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FontInfo, familyName, subfamilyName);

class Presenter : public std::enable_shared_from_this<Presenter>
{
  std::shared_ptr<UIScrollView> m_view;
  std::shared_ptr<ViewModel>    m_viewModel;
  bool                          m_listenAllEvents{ false };

  std::vector<FontInfo> m_requiredFonts;

  std::shared_ptr<UIView>    m_editView;
  std::shared_ptr<ViewModel> m_editViewModel;

  std::shared_ptr<Mouse> m_mouse;

  bool                  m_editMode{ false };
  UIView::EventListener m_editorEventListener;

  rxcpp::subjects::subject<VGG::UIEventPtr> m_subject;
  rxcpp::subjects::subject<VGG::UIEventPtr> m_editSubject;
  rxcpp::observer<VGG::ModelEventPtr>       m_modelObserver;
  rxcpp::observer<VGG::ModelEventPtr>       m_editModelObserver;

public:
  Presenter(std::shared_ptr<Mouse> mouse = nullptr)
    : m_mouse{ mouse }
  {
  }

  virtual ~Presenter() = default;

  bool handleTranslate(double pageWidth, double pageHeight, float x, float y);

  void setListenAllEvents(bool enabled)
  {
    m_listenAllEvents = enabled;
  }

  void fitForEditing(Layout::Size pageSize);
  void resetForRunning();

  virtual int currentPageIndex()
  {
    return m_view->currentPageIndex();
  }
  bool setCurrentPage(int index)
  {
    return m_view->setCurrentPage(index);
  }
  bool presentPage(int index);
  bool dismissPage();
  bool goBack(bool resetScrollPosition, bool resetState);

  void                       saveState(std::shared_ptr<StateTree> stateTree);
  std::shared_ptr<StateTree> savedState();
  void                       restoreState();

  virtual void setModel(std::shared_ptr<ViewModel> viewModel);
  auto         requiredFonts()
  {
    return m_requiredFonts;
  }

  void setNeedsReload()
  {
    m_view->setDirty(true);
  }

  void setEditMode(bool editMode)
  {
    m_editMode = editMode;
  }

  void setEditModel(std::shared_ptr<ViewModel> viewModel)
  {
    m_editViewModel = viewModel;
    if (m_editView)
    {
      m_editView->show(*m_editViewModel);

      m_editView->registerEventListener(
        [viewModel](std::string path, EUIEventType eventType)
        {
          switch (eventType)
          {
            case EUIEventType::CLICK:
              return true;

            default:
              return false;
          }
        });
    }
    else
    {
      WARN("Presenter::setEditModel: empty m_editView");
    }
  }

  void setView(std::shared_ptr<UIScrollView> view);

  void setEditorEventListener(UIView::EventListener listener)
  {
    m_editorEventListener = listener;
  }

  Layout::Size viewSize()
  {
    if (m_view)
    {
      return m_view->size();
    }
    else
    {
      return {};
    }
  }

  void setContentSize(Layout::Size size)
  {
    if (m_view)
    {
      m_view->setContentSize(size);
    }
  }

  void setEditView(std::shared_ptr<UIView> view)
  {
    m_editView = view;

    auto weakThis = weak_from_this();
    m_editView->setEventListener(
      [weakThis](UIEventPtr evtPtr, std::weak_ptr<LayoutNode> targetNode)
      {
        if (auto p = weakThis.lock())
        {
          p->m_editSubject.get_subscriber().on_next(evtPtr);
        }
      });
  }

  Layout::Size editViewSize()
  {
    if (m_editView)
    {
      return m_editView->size();
    }
    else
    {
      return {};
    }
  }

  virtual rxcpp::observer<VGG::ModelEventPtr>& getModelObserver()
  {
    auto weakThis = weak_from_this();
    m_modelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
      [weakThis](ModelEventPtr event)
      {
        auto strongThis = weakThis.lock();
        if (!strongThis || !event)
        {
          return;
        }

        // todo, diff & update
        strongThis->update();
      });

    return m_modelObserver;
  }

  virtual rxcpp::observer<VGG::ModelEventPtr>& getEditModelObserver()
  {
    auto weakThis = weak_from_this();
    m_editModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
      [weakThis](ModelEventPtr event)
      {
        if (auto p = weakThis.lock())
        {
          // todo, diff & update
          p->updateEditView();
        }
      });

    return m_editModelObserver;
  }

  virtual rxcpp::observable<VGG::UIEventPtr> getObservable()
  {
    return m_subject.get_observable();
  }

  virtual rxcpp::observable<VGG::UIEventPtr> getEditObservable()
  {
    return m_editSubject.get_observable();
  }

  void update();

private:
  void listenViewEvent();

  void updateEditView()
  {
    if (m_editView && m_editViewModel)
    {
      m_editView->show(*m_editViewModel);
    }
  }
};

} // namespace VGG
