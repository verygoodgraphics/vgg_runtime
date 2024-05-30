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

#include "Layer/Core/VNode.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Core/RenderNode.hpp"

namespace VGG::layer
{

class TransformNode : public VNode
{
public:
  TransformNode(VRefCnt* cnt)
    : VNode(cnt)
  {
  }

  virtual glm::mat3 getMatrix() const = 0;
  virtual glm::mat3 getInversedMatrix()
  {
    return glm::inverse(getMatrix());
  }
};

class TransformEffectNode : public RenderNode
{
public:
  TransformEffectNode(VRefCnt* cnt, Ref<TransformNode> transform, Ref<RenderNode> child)
    : RenderNode(cnt, EState::INVALIDATE)
    , m_transform(std::move(transform))
    , m_child(std::move(child))
  {
    ASSERT(m_transform);
    ASSERT(m_child);
    observe(m_transform);
    observe(m_child);
  }

  Bounds effectBounds() const override
  {
    return m_child->effectBounds();
  }

  void nodeAt(int x, int y, NodeVisitor visitor, void* userData) override
  {
    if (m_child)
    {
      if (!m_transform)
      {
        const auto fp = m_transform->getInversedMatrix() * glm::vec3{ x, y, 1 };
        x = fp.x;
        y = fp.y;
        return getChild()->nodeAt(x, y, visitor, userData);
      }
      return getChild()->nodeAt(x, y, visitor, userData);
    }
  }

  void render(Renderer* renderer) override;

  ~TransformEffectNode() override
  {
    unobserve(m_transform);
    unobserve(m_child);
  }

  Bounds onRevalidate(Revalidation* inv, const glm::mat3& ctm) override
  {
    Bounds bounds;
    if (m_child)
    {
      if (m_transform)
      {
        m_transform->revalidate();
        const auto matrix = m_transform->getMatrix();
        bounds = m_child->revalidate(inv, matrix * ctm);
        bounds = bounds.bounds(Transform(matrix));
      }
      else
      {
        bounds = m_child->revalidate(inv, ctm);
      }
    }
    return bounds;
  }

  VGG_CLASS_MAKE(TransformEffectNode);

protected:
  TransformNode* getTransform() const
  {
    return m_transform.get();
  }

  RenderNode* getChild() const
  {
    return m_child.get();
  }

private:
  Ref<TransformNode> m_transform;
  Ref<RenderNode>    m_child;
};

} // namespace VGG::layer
