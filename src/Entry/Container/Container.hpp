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

#include "Event.hpp"
#include "IContainer.hpp"
#include "ISdk.hpp"
#include "VggTypes.hpp"

#include <memory>

namespace VGG
{

namespace layer
{
class SkiaGraphicsContext;
}

class ContainerImpl;
class VGG_RUNTIME_DLL_DECLARE Container : public IContainer
{
  std::shared_ptr<ContainerImpl> m_impl;

public:
  Container();
  ~Container();

  void setGraphicsContext(std::unique_ptr<layer::SkiaGraphicsContext>& context, int w, int h);

private:
  virtual IContainer* container() override;
};

} // namespace VGG