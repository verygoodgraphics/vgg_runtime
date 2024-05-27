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

#include "Pager.hpp"
#include <cstddef>
#include <vector>
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Memory/Ref.hpp"

namespace VGG::internal
{

Pager::Pager(layer::SceneNode* sceneNode)
  : m_sceneNode(sceneNode)
{
  if (m_sceneNode && !m_sceneNode->getFrames().empty())
  {
    const auto& frames = m_sceneNode->getFrames();
    for (std::size_t i = 0; i < m_sceneNode->getFrames().size(); i++)
    {
      frames[i]->node()->setVisible(false);
    }
    m_currentPage = 0;
    frames[m_currentPage]->node()->setVisible(true);
  }
  else
  {
    m_currentPage = -1;
  }
}

Pager::~Pager() = default;

Bounds Pager::getPageBounds()
{
  if (
    m_currentPage < 0 || static_cast<std::size_t>(m_currentPage) >= m_sceneNode->getFrames().size())
  {
    return Bounds();
  }
  const auto& frames = m_sceneNode->getFrames();
  return frames[m_currentPage]->bounds();
}

layer::FrameNode* Pager::currentFrame()
{
  if (
    m_currentPage < 0 || static_cast<std::size_t>(m_currentPage) >= m_sceneNode->getFrames().size())
  {
    return nullptr;
  }
  return m_sceneNode->getFrames()[m_currentPage].get();
}

int Pager::page()
{
  return m_currentPage;
}

void Pager::setPage(int index)
{
  setPageOffset(index - m_currentPage);
}

void Pager::nextFrame()
{
  setPageOffset(1);
}

void Pager::prevFrame()
{
  setPageOffset(-1);
}

void Pager::setPageOffset(int delta)
{
  if (m_sceneNode && !m_sceneNode->getFrames().empty())
  {
    const int total = m_sceneNode->getFrames().size();
    auto      newPage = (m_currentPage + delta + total) % total;
    if (newPage == m_currentPage)
      return;
    const auto& frames = m_sceneNode->getFrames();
    frames[m_currentPage]->node()->setVisible(false);
    frames[newPage]->node()->setVisible(true);
    m_currentPage = newPage;
  }
}

} // namespace VGG::internal