#pragma once

#include "VggJSEngine.hpp"

#include "gmock/gmock.h"

class VggJSEngineMock : public VggJSEngine
{
public:
  MOCK_METHOD(bool, eval, (std::string_view buffer, const char* filename, int flags), (override));
};
