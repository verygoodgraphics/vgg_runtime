#pragma once

#include "Domain/ModelEvent.hpp"
#include "Domain/Daruma.hpp"
#include "Log.h"
#include "UIEvent.hpp"
#include "UIView.hpp"

#include "nlohmann/json.hpp"
#include "rxcpp/rx.hpp"

#include <memory>

namespace VGG
{

class Presenter : public std::enable_shared_from_this<Presenter>
{
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<Daruma> m_model;

  std::shared_ptr<UIView> m_edit_view;
  std::shared_ptr<Daruma> m_edit_model;

  rxcpp::subjects::subject<VGG::UIEventPtr> m_subject;
  rxcpp::observer<VGG::ModelEventPtr> m_model_observer;
  rxcpp::observer<VGG::ModelEventPtr> m_edit_model_observer;

public:
  virtual ~Presenter() = default;

  virtual void setModel(std::shared_ptr<Daruma> model)
  {
    m_model = model;
    m_view->setResouces(m_model->resources());
    m_view->show(m_model->designDoc()->content());
  }

  void setEditModel(std::shared_ptr<Daruma> model)
  {
    m_edit_model = model;
    if (m_edit_view)
    {
      m_edit_view->setResouces(m_edit_model->resources());
      m_edit_view->show(m_edit_model->designDoc()->content());
    }
    else
    {
      WARN("Presenter::setEditModel: empty m_edit_view");
    }
  }

  void setView(std::shared_ptr<UIView> view)
  {
    m_view = view;

    auto weak_this = weak_from_this();
    m_view->setEventListener(
      [weak_this](UIEventPtr evt_ptr)
      {
        if (auto p = weak_this.lock())
        {
          p->m_subject.get_subscriber().on_next(evt_ptr);
        }
      });
  }

  void setEditView(std::shared_ptr<UIView> view)
  {
    m_edit_view = view;
  }

  virtual rxcpp::observer<VGG::ModelEventPtr>& getModelObserver()
  {
    auto weak_this = weak_from_this();
    m_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
      [weak_this](ModelEventPtr event)
      {
        switch (event->type)
        {
          case ModelEventType::Add:
          {
            auto add_event_ptr = static_cast<ModelEventAdd*>(event.get());
          }
          break;

          case ModelEventType::Delete:
          {
            auto delete_event_ptr = static_cast<ModelEventDelete*>(event.get());
          }
          break;

          case ModelEventType::Update:
          {
            auto udpate_event_ptr = static_cast<ModelEventUpdate*>(event.get());
          }
          break;

          default:
            break;
        }

        if (auto p = weak_this.lock())
        {
          // todo, diff & update
          p->update();
        }
      });

    return m_model_observer;
  }

  virtual rxcpp::observer<VGG::ModelEventPtr>& getEditModelObserver()
  {
    auto weak_this = weak_from_this();
    m_edit_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
      [weak_this](ModelEventPtr event)
      {
        switch (event->type)
        {
          case ModelEventType::Add:
          {
            auto add_event_ptr = static_cast<ModelEventAdd*>(event.get());
          }
          break;

          case ModelEventType::Delete:
          {
            auto delete_event_ptr = static_cast<ModelEventDelete*>(event.get());
          }
          break;

          case ModelEventType::Update:
          {
            auto udpate_event_ptr = static_cast<ModelEventUpdate*>(event.get());
          }
          break;

          default:
            break;
        }

        if (auto p = weak_this.lock())
        {
          // todo, diff & update
          p->update_edit_view();
        }
      });

    return m_edit_model_observer;
  }

  virtual rxcpp::observable<VGG::UIEventPtr> getObservable()
  {
    return m_subject.get_observable();
  }

private:
  void update()
  {
    m_view->show(m_model->designDoc()->content());
  }

  void update_edit_view()
  {
    if (m_edit_view && m_edit_model)
    {
      m_edit_view->show(m_edit_model->designDoc()->content());
    }
  }
};

} // namespace VGG
