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

#include <string>
#include <memory>

namespace VGG
{
class LayoutNode;
class StateTree;

class InstanceState
{
  std::shared_ptr<LayoutNode> m_page;

public:
  InstanceState(std::shared_ptr<LayoutNode> page);

  bool presentState(
    const std::string& instanceDescendantId,
    const std::string& stateMasterId,
    StateTree*         stateTree);
  bool dismissState(const std::string& instanceDescendantId);
  bool setMasterId(const std::string& instanceDescendantId, const std::string& stateMasterId);

private:
  std::shared_ptr<LayoutNode> findInstanceNode(const std::string& instanceDescendantId);
};

} // namespace VGG
