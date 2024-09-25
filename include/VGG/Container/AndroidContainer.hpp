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

struct ANativeWindow;
typedef struct ANativeWindow ANativeWindow;

namespace VGG
{

class AndroidContainerImpl;
class AndroidContainer : public IContainer
{
  std::unique_ptr<AndroidContainerImpl> m_impl;

public:

  AndroidContainer();
  ~AndroidContainer();

  void setView(ANativeWindow* window);

private:
  virtual IContainer* container() override;
};

} // namespace VGG
