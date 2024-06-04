/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "IContainer.hpp"
#include <memory>

namespace VGG
{

class QtQuickContainerImpl;

class VGG_RUNTIME_DLL_DECLARE QtQuickContainer : public IContainer
{
public:
  QtQuickContainer(
    int          width,
    int          height,
    double       devicePixelRatio,
    unsigned int fboID,
    int          stencilBit = 8,
    int          multiSample = 4);

  ~QtQuickContainer();

private:
  virtual IContainer* container() override;

private:
  std::unique_ptr<QtQuickContainerImpl> m_impl;
};

} // namespace VGG