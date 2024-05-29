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
};

class TransformEffectNode : public RenderNode
{
public:
  TransformEffectNode(VRefCnt* cnt, Ref<TransformNode> transform, Ref<RenderNode> child)
    : RenderNode(cnt, EState::INVALIDATE)
    , m_transform(std::move(transform))
    , m_child(std::move(child))
  {
    observe(m_transform);
    observe(m_child);
  }

  Bounds effectBounds() const override
  {
    return m_child->effectBounds();
  }

  void render(Renderer* renderer) override;

  ~TransformEffectNode() override
  {
    unobserve(m_transform);
    unobserve(m_child);
  }

  Bounds onRevalidate(Invalidator* inv, const glm::mat3 & mat) override
  {
    if (m_transform)
      m_transform->revalidate();
    Bounds bounds;
    if (m_child)
    {
      bounds = m_child->revalidate();
      if (m_transform)
      {
        m_transform->revalidate();
        bounds = bounds.bounds(Transform(m_transform->getMatrix()));
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
