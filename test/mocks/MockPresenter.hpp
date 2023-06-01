#pragma once

#include "Presenter/Presenter.hpp"

#include "gmock/gmock.h"

class MockPresenter : public VGG::Presenter
{
public:
  MOCK_METHOD(rxcpp::observer<VGG::ModelEventPtr>&, getDesignDocObserver, (), (override));
  MOCK_METHOD(rxcpp::observable<VGG::ViewEventPtr>, getObservable, (), (override));
};
