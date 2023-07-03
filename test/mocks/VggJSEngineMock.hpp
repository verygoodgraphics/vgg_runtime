#pragma once

#include "Exec/VggJSEngine.hpp"

#include "gmock/gmock.h"

class VggJSEngineMock : public VggJSEngine
{
public:
  MOCK_METHOD(bool, evalScript, (const std::string& code), (override));
  MOCK_METHOD(bool, evalModule, (const std::string& code, VGG::EventPtr event), (override));
};
