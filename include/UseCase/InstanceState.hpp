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
class SnapshotTree;

namespace Layout
{
class ExpandSymbol;
class Layout;
} // namespace Layout

class InstanceState
{
  std::shared_ptr<LayoutNode>           m_page;
  std::shared_ptr<Layout::ExpandSymbol> m_expander;
  std::shared_ptr<Layout::Layout>       m_layout;

public:
  struct Result
  {
    bool                          success{ false };
    std::shared_ptr<SnapshotTree> oldTree;
    std::shared_ptr<LayoutNode>   newTree;
  };

public:
  InstanceState(
    std::shared_ptr<LayoutNode>           page,
    std::shared_ptr<Layout::ExpandSymbol> expander,
    std::shared_ptr<Layout::Layout>       layout);

  Result setState(
    const std::string& instanceDescendantId,
    const std::string& listenerId,
    const std::string& stateMasterId);
  Result presentState(
    const std::string& instanceDescendantId,
    const std::string& listenerId,
    const std::string& stateMasterId,
    StateTree*         stateTree);
  Result dismissState(const StateTree* savedStateTree, const std::string& instanceDescendantId);

private:
  Result setState(std::shared_ptr<LayoutNode> instanceNode, const std::string& stateMasterId);
  std::shared_ptr<LayoutNode> findInstanceNode(
    const std::string& instanceDescendantId,
    const std::string& listenerId);
};

} // namespace VGG
