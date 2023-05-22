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
      [&](ModelEventPtr evt)
      {
        auto type = evt->type;
        switch (type)
        {
          case ModelEventType::Add:
          {
            const auto& add_evt = std::any_cast<ModelEventAdd>(evt->data);
          }
          break;

          case ModelEventType::Delete:
          {
            const auto& delete_evt = std::any_cast<ModelEventDelete>(evt->data);
          }
          break;

          case ModelEventType::Update:
          {
            const auto& udpate_evt = std::any_cast<ModelEventUpdate>(evt->data);
          }
          break;

          default:
            break;
        }
      });

    m_layout_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
      [&](ModelEventPtr evt)
      {
        auto type = evt->type;
        auto data = evt->data;
      });
  }

  virtual ~Presenter() = default;

  virtual rxcpp::observer<VGG::ModelEventPtr> getDesignDocObserver()
  {
    return m_design_doc_observer;
  }

  virtual rxcpp::observer<VGG::ModelEventPtr> getLayoutDocObserver()
  {
    return m_layout_doc_observer;
  }

  virtual rxcpp::observable<VGG::ViewEventPtr> getObservable()
  {
    return m_subject.get_observable();
  }
};

} // namespace VGG
