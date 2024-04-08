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
#include "ISdk.hpp"
#include "VggTypes.hpp"
#include "VggPackage.hpp"

#include <memory>

namespace VGG
{

class VGG_RUNTIME_DLL_DECLARE IContainer
{
protected:
  virtual IContainer* container() = 0;

public:
  virtual ~IContainer() = default;

  virtual bool load(
    const std::string& filePath,
    const char*        designDocSchemaFilePath = nullptr,
    const char*        layoutDocSchemaFilePath = nullptr)
  {
    return container()->load(filePath, designDocSchemaFilePath, layoutDocSchemaFilePath);
  }

  void run()
  {
    container()->paint();
    container()->dispatch();
  }

  virtual bool needsPaint()
  {
    return container()->needsPaint();
  }

  virtual bool paint(bool force = false)
  {
    return container()->paint(force);
  }

  virtual void dispatch()
  {
    return container()->dispatch();
  }

  virtual bool onEvent(UEvent evt)
  {
    return container()->onEvent(evt);
  }
  virtual void setEventListener(EventListener listener)
  {
    return container()->setEventListener(listener);
  }

  virtual std::shared_ptr<ISdk> sdk()
  {
    return container()->sdk();
  }
};

} // namespace VGG