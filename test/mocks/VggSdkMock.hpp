#pragma once

#include "Sdk/VggSdk.hpp"

#include "gmock/gmock.h"

#include <iostream>

class VggSdkMock : public VggSdk
{
public:
  // MOCK_METHOD(const std::string, documentJson, (), (const));
  // MOCK_METHOD(const std::string, findElement, (const std::string&), (const));

  MOCK_METHOD(void, updateStyle, (), (override));
};
