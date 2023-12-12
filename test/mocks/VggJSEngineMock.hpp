#pragma once

#include "Domain/VggJSEngine.hpp"

#include "gmock/gmock.h"

class VggJSEngineMock : public VGG::VggJSEngine
{
public:
  MOCK_METHOD(bool, evalScript, (const std::string& code), (override));
  MOCK_METHOD(bool, evalModule, (const std::string& code), (override));
  MOCK_METHOD(
    bool,
    evalModule,
    (const std::string& code, VGG::EventPtr event, std::shared_ptr<VGG::IVggEnv> env),
    (override));
};
