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

#include "IContainer.hpp"

#include <memory>

namespace VGG
{

class QtContainerImpl;
class VGG_RUNTIME_DLL_DECLARE QtContainer : public IContainer
{
  std::unique_ptr<QtContainerImpl> m_impl;

public:
  QtContainer();
  ~QtContainer();

  void init(int w, int h, float devicePixelRatio);

private:
  virtual IContainer* container() override;
};

} // namespace VGG