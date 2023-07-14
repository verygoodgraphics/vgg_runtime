#pragma once

#include "Application/Presenter.hpp"

#include "gmock/gmock.h"

class MockPresenter : public VGG::Presenter
{
public:
  MOCK_METHOD(rxcpp::observer<VGG::ModelEventPtr>&, getModelObserver, (), (override));
  MOCK_METHOD(rxcpp::observable<VGG::UIEventPtr>, getObservable, (), (override));
  MOCK_METHOD(void, setModel, (std::shared_ptr<VGG::Daruma>), (override));
};
