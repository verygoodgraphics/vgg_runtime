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

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Application/UIAnimation.hpp"
#include "Application/UIEvent.hpp"
#include "Application/UIOptions.hpp"
#include "Application/UIScrollView.hpp"
#include "Application/UIUpdateElement.hpp"
#include "Application/UIView.hpp"
#include "Domain/Layout/Rect.hpp"
#include "Domain/ModelEvent.hpp"
#include "Utility/Log.hpp"
#include "ViewModel.hpp"
#include <nlohmann/json.hpp>
#include <rxcpp/rx-includes.hpp>
#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-observer.hpp>
#include <rxcpp/rx-operators.hpp>
#include <rxcpp/rx-predef.hpp>
#include <rxcpp/rx-sources.hpp>
#include <rxcpp/subjects/rx-subject.hpp>
namespace VGG
{
class LayoutContext;
class LayoutNode;
class Mouse;
class StateTree;
} // namespace VGG

namespace VGG
{

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
  app::UIAnimationOption m_lastPushFrameAnimationOptions;
  app::FrameOptions      m_lastPresentFrameOptions;

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
  void setBackgroundColor(uint32_t color);

  virtual int currentPageIndex()
  {
    return m_view->currentPageIndex();
  }
  bool setCurrentFrameIndex(const std::size_t index, const bool updateHistory);
  bool pushFrame(
    const std::size_t              index,
    const bool                     updateHistory,
    const app::FrameOptions&       option = {},
    const app::AnimationCompletion completion = {});
  bool presentFrame(
    const std::size_t        index,
    const app::FrameOptions& opts,
    app::AnimationCompletion completion);
  bool dismissFrame(app::AnimationCompletion completion);
  void initHistory();
  bool popFrame(const app::PopOptions& opts, app::AnimationCompletion completion);

  void triggerMouseEnter();

  virtual void setModel(std::shared_ptr<ViewModel> viewModel);
  auto         requiredFonts()
  {
    return m_requiredFonts;
  }

  void setDirtry()
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

  std::shared_ptr<StateTree>    savedState(const std::string& instanceDescendantId);
  const app::UIAnimationOption& dismissStateAnimationOptions() const
  {
    return m_lastPresentStateAnimationOptions;
  }

public:
  int updateElement(
    const std::vector<app::UpdateElementItem>& items,
    const app::UIAnimationOption&              option);

  bool setElementFillEnabled(
    const std::string&            id,
    std::size_t                   index,
    bool                          enabled,
    const app::UIAnimationOption& animation);
  bool setElementFillColor(
    const std::string&            id,
    std::size_t                   index,
    float                         a,
    float                         r,
    float                         g,
    float                         b,
    const app::UIAnimationOption& animation);
  bool setElementFillOpacity(
    const std::string&            id,
    std::size_t                   index,
    float                         opacity,
    const app::UIAnimationOption& animation);
  bool setElementFillBlendMode(
    const std::string&            id,
    std::size_t                   index,
    int                           mode,
    const app::UIAnimationOption& animation);
  bool setElementFillRotation(
    const std::string&            id,
    std::size_t                   index,
    float                         degree,
    const app::UIAnimationOption& animation);

  bool setElementOpacity(
    const std::string&            id,
    float                         opacity,
    const app::UIAnimationOption& animation);
  bool setElementVisible(
    const std::string&            id,
    bool                          visible,
    const app::UIAnimationOption& animation);
  bool setElementMatrix(
    const std::string&            id,
    float                         a,
    float                         b,
    float                         c,
    float                         d,
    float                         tx,
    float                         ty,
    const app::UIAnimationOption& animation);
  bool setElementSize(
    const std::string&            id,
    float                         width,
    float                         height,
    const app::UIAnimationOption& animation);

public:
  bool updateViewNodeFillColor(
    const std::string& id,
    const std::size_t  fillIndex,
    const double       r,
    const double       g,
    const double       b,
    const double       a);

  std::unique_ptr<LayoutContext> layoutContext();

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
