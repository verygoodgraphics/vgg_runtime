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

#include "Application/UIAnimation.hpp"
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
public:
  enum class EContentMode
  {
    TOP_LEFT,
    SCALE_ASPECT_FIT,
    SCALE_ASPECT_FILL,
    SCALE_ASPECT_FILL_TOP_CENTER
  };

private:
  std::shared_ptr<UIScrollView> m_view;
  std::shared_ptr<ViewModel>    m_viewModel;
  bool                          m_listenAllEvents{ false };

  std::vector<FontInfo> m_requiredFonts;

  std::shared_ptr<UIView>    m_editView;
  std::shared_ptr<ViewModel> m_editViewModel;

  std::shared_ptr<Mouse> m_mouse;

  EContentMode          m_contentMode{ EContentMode::TOP_LEFT };
  bool                  m_editMode{ false };
  UIView::EventListener m_editorEventListener;

  rxcpp::subjects::subject<VGG::UIEventPtr> m_subject;
  rxcpp::subjects::subject<VGG::UIEventPtr> m_editSubject;
  rxcpp::observer<VGG::ModelEventPtr>       m_modelObserver;
  rxcpp::observer<VGG::ModelEventPtr>       m_editModelObserver;

  app::UIAnimationOption m_lastPresentStateAnimationOptions;
  app::UIAnimationOption m_lastPresentPageAnimationOptions;

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

  void fitForEditing(const Layout::Size& pageSize);
  void fitForRunning(const Layout::Size& pageSize);

  const auto contentMode() const
  {
    return m_contentMode;
  }
  void setContentMode(const EContentMode mode)
  {
    m_contentMode = mode;
  }

  virtual int currentPageIndex()
  {
    return m_view->currentPageIndex();
  }
  bool setCurrentPageIndex(std::size_t index, bool updateHistory);
  bool setCurrentPageIndexAnimated(
    std::size_t                   index,
    const app::UIAnimationOption& option,
    app::AnimationCompletion      completion = app::AnimationCompletion());
  bool presentPage(int index);
  bool dismissPage();
  void initHistory();
  bool goBack(bool resetScrollPosition, bool resetState);

  std::shared_ptr<StateTree> savedState();

  void triggerMouseEnter();

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
      m_editView->show(m_editViewModel);

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

  auto getView()
  {
    return this->m_view;
  }

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

public:
  bool setInstanceState(
    const LayoutNode*             oldNode,
    const LayoutNode*             newNode,
    const app::UIAnimationOption& options,
    app::AnimationCompletion      completion);
  bool presentInstanceState(
    const std::shared_ptr<StateTree>& oldState,
    const LayoutNode*                 oldNode,
    const LayoutNode*                 newNode,
    const app::UIAnimationOption&     options,
    app::AnimationCompletion          completion);

  const app::UIAnimationOption& dismissStateAnimationOptions() const
  {
    return m_lastPresentStateAnimationOptions;
  }
  const app::UIAnimationOption& dismissPageAnimationOptions() const
  {
    return m_lastPresentPageAnimationOptions;
  }

public:
  bool updateViewNodeFillColor(
    const std::string& id,
    const std::size_t  fillIndex,
    const double       r,
    const double       g,
    const double       b,
    const double       a);

private:
  void listenViewEvent();

  void updateEditView()
  {
    if (m_editView && m_editViewModel)
    {
      m_editView->show(m_editViewModel);
    }
  }

  void fitForAspectScale(const Layout::Size& pageSize);
};

} // namespace VGG
