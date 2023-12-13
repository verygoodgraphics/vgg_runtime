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

#include "Domain/DarumaContainer.hpp"

#include <string>

namespace VGG
{

class IVggEnv
{
public:
  virtual ~IVggEnv() = default;

  virtual std::string getEnv() = 0;

  virtual std::string getContainerKey() = 0;
  virtual void        setContainerKey(const std::string& containerKey) = 0;

  virtual std::string getInstanceKey() = 0;
  virtual void        setInstanceKey(const std::string& instanceKey) = 0;

  virtual std::string getListenerKey() = 0;
  virtual void        setListenerKey(const std::string& listenerKey) = 0;

  std::string currrentEnvName()
  {
    return "_currentVggEnv";
  }
  std::string currrentVggName()
  {
    return "_currentVgg";
  }

  virtual DarumaContainer& darumaContainer() = 0;
};

} // namespace VGG