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

#include "AppLayoutContext.hpp"
#include <memory>
#include "Layer/Core/PaintNode.hpp"
#include "Utility/Log.hpp"

namespace VGG
{

AppLayoutContext::AppLayoutContext(std::shared_ptr<AttrBridge> layerBridge)
  : m_layerBridge(layerBridge)
{
  ASSERT(m_layerBridge);
}

std::optional<Layout::Size> AppLayoutContext::nodeSize(LayoutNode* node) const
{
  ASSERT(node);
  auto paintNode = m_layerBridge->getPaintNode(node->shared_from_this());
  if (!paintNode) // scene node not ready
    return {};

  paintNode->revalidate();
  auto& b = paintNode->bounds();
  return Layout::Size{ b.width(), b.height() };
}

void AppLayoutContext::didUpdateBounds(LayoutNode* node)
{
  ASSERT(node);
  auto paintNode = m_layerBridge->getPaintNode(node->shared_from_this());
  if (!paintNode) // scene node not ready
    return;
  const auto size = node->bounds().size;
  m_layerBridge->updateSize(node->shared_from_this(), paintNode, size.width, size.height, true);
}

void AppLayoutContext::didUpdateMatrix(LayoutNode* node)
{
  ASSERT(node);
  auto paintNode = m_layerBridge->getPaintNode(node->shared_from_this());
  if (!paintNode) // scene node not ready
    return;

  const auto&   m = node->modelMatrix();
  TDesignMatrix newMatrix{ m.a, m.b, m.c, m.d, m.tx, m.ty };
  m_layerBridge->updateMatrix(node->shared_from_this(), paintNode, newMatrix, true);
}

void AppLayoutContext::didUpdateContourPoints(LayoutNode* node)
{
  ASSERT(node);
}

} // namespace VGG