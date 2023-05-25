#pragma once

#include "Model/ModelEvent.hpp"
#include "UIEvent.hpp"

#include "rxcpp/rx.hpp"

#include <memory>

namespace VGG
{

class Presenter
{
  rxcpp::subjects::subject<VGG::UIEventPtr> m_subject;
  rxcpp::observer<VGG::ModelEventPtr> m_model_observer;

public:
  Presenter()
  {
    m_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
      [&](ModelEventPtr event)
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
      });
  }

  virtual ~Presenter() = default;

  virtual rxcpp::observer<VGG::ModelEventPtr>& getModelObserver()
  {
    return m_model_observer;
  }

  virtual rxcpp::observable<VGG::UIEventPtr> getObservable()
  {
    return m_subject.get_observable();
  }
};

} // namespace VGG
