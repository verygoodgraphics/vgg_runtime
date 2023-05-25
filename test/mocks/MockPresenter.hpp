#pragma once

#include "Presenter/Presenter.hpp"

#include "gmock/gmock.h"

class MockPresenter : public VGG::Presenter
{
public:
  MOCK_METHOD(rxcpp::observer<VGG::ModelEventPtr>&, getModelObserver, (), (override));
  MOCK_METHOD(rxcpp::observable<VGG::UIEventPtr>, getObservable, (), (override));
};
