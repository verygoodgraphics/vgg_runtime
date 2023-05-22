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
