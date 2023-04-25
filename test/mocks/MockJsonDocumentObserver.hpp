#pragma once

#include "Model/SubjectJsonDocument.hpp"

#include "gmock/gmock.h"

class MockJsonDocumentObserver : public JsonDocumentObserver
{
public:
  MOCK_METHOD(void, didAdd, (const json::json_pointer& path, const json& value), (override));
  MOCK_METHOD(void, didUpdate, (const json::json_pointer& path, const json& value), (override));
  MOCK_METHOD(void, didDelete, (const json::json_pointer& path), (override));
};
