#pragma once

#include "../Model/ModelEvent.hpp"

#include "rxcpp/rx.hpp"

#include <memory>

namespace VGG
{

struct ViewEvent
{
};
using ViewEventPtr = std::shared_ptr<ViewEvent>;

class Presenter
{
  rxcpp::subjects::subject<VGG::ViewEventPtr> m_subject;
  rxcpp::observer<VGG::ModelEventPtr> m_observer;

public:
  auto getObserver()
  {
    return m_observer;
  }

  auto getObservable()
  {
    return m_subject.get_observable();
  }
};

} // namespace VGG
