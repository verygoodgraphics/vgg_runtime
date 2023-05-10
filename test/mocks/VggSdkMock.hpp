#pragma once

#include "Sdk/VggSdk.hpp"

#include "gmock/gmock.h"

#include <iostream>

class VggSdkMock : public VggSdk
{
public:
  MOCK_METHOD(void, updateStyle, (), ());
};
