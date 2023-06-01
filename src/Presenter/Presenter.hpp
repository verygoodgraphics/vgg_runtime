#pragma once

#include "Model/ModelEvent.hpp"
#include "ViewEvent.hpp"

#include "rxcpp/rx.hpp"

#include <memory>

namespace VGG
{

class Presenter
{
  rxcpp::subjects::subject<VGG::ViewEventPtr> m_subject;
  rxcpp::observer<VGG::ModelEventPtr> m_design_doc_observer;
  rxcpp::observer<VGG::ModelEventPtr> m_layout_doc_observer;

public:
  Presenter()
  {
    m_design_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
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

    m_layout_doc_observer =
      rxcpp::make_observer_dynamic<ModelEventPtr>([&](ModelEventPtr event) {});
  }

  virtual ~Presenter() = default;

  virtual rxcpp::observer<VGG::ModelEventPtr>& getDesignDocObserver()
  {
    return m_design_doc_observer;
  }

  virtual rxcpp::observer<VGG::ModelEventPtr>& getLayoutDocObserver()
  {
    return m_layout_doc_observer;
  }

  virtual rxcpp::observable<VGG::ViewEventPtr> getObservable()
  {
    return m_subject.get_observable();
  }
};

} // namespace VGG
