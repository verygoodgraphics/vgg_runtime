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

#include <memory>
#include "Domain/Daruma.hpp"
#include "Domain/Layout/Layout.hpp"
namespace VGG
{
class LayoutNode;
namespace Layout
{
class ExpandSymbol;
}
} // namespace VGG

namespace VGG
{

class StartRunning
{
  std::shared_ptr<Layout::Layout>       m_layout;
  std::shared_ptr<Layout::ExpandSymbol> m_expander;

public:
  StartRunning(std::shared_ptr<Daruma> model);

  std::shared_ptr<Layout::Layout> layout()
  {
    return m_layout;
  }

  std::shared_ptr<LayoutNode> layoutTree()
  {
    return m_layout->layoutTree();
  }

  auto expander()
  {
    return m_expander;
  }
};

} // namespace VGG
