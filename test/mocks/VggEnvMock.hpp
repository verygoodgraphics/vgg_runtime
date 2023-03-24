#pragma once

#include "VggEnv.hpp"

#include "gmock/gmock.h"

class VggEnvMock : public VggEnv
{
public:
  MOCK_METHOD(const std::string, getEnv, (), (override));
};
