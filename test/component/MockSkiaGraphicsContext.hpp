/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Layer/Graphics/ContextSkBase.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"

#include "gmock/gmock.h"

class MockSkiaGraphicsContext : public VGG::layer::SkiaGraphicsContext
{
public:
  // MOCK_METHOD(bool, swap, (), (override));
  // MOCK_METHOD(bool, makeCurrent, (), (override));
  // MOCK_METHOD(void, shutdown, (), (override));
  // MOCK_METHOD(void*, contextInfo, (), (override));

  virtual bool swap() override
  {
    return false;
  }

  virtual bool makeCurrent() override
  {
    return false;
  }

  virtual void shutdown() override
  {
    return;
  }

  virtual void* contextInfo() override
  {
    return nullptr;
  }

  // MOCK_METHOD(bool, onInit, (), (override));
  virtual bool onInit() override
  {
    return false;
  }

  // MOCK_METHOD(SurfaceCreateProc, surfaceCreateProc, (), (override));
  // MOCK_METHOD(ContextCreateProc, contextCreateProc, (), (override));

  virtual SurfaceCreateProc surfaceCreateProc() override;
  virtual ContextCreateProc contextCreateProc() override;

  virtual void onInitProperties(VGG::layer::ContextProperty& property) override
  {
    property.dpiScaling = 2; // mock
    property.api = VGG::layer::EGraphicsAPIBackend::API_METAL;
  }
};
