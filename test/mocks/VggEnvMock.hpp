#pragma once

#include "Exec/IVggEnv.hpp"

#include "gmock/gmock.h"

class VggEnvMock : public IVggEnv
{
public:
  MOCK_METHOD(const std::string, getEnv, (), (override));
};
