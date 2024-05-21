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

#include "Layer/Core/RenderNode.hpp"
#include "Layer/Core/Attrs.hpp"
#include <complex>

namespace VGG::layer
{

class EffectNode : public RenderNode
{
public:
  EffectNode(VRefCnt* cnt, Ref<RenderNode> child)
    : RenderNode(cnt, EState::INVALIDATE)
    , m_child(std::move(child))
  {
    observe(m_child);
  }

  Ref<RenderNode> getChild() const
  {
    return m_child;
  }

  virtual Bounds effectBounds() const override = 0;

  ~EffectNode()
  {
    unobserve(m_child);
  }

private:
  Ref<RenderNode> m_child;
};

class FillEffectNode : public EffectNode
{
public:
  FillEffectNode(VRefCnt* cnt, Ref<RenderNode> child)
    : EffectNode(cnt, std::move(child))
  {
  }
};

class BorderEffectNode : public EffectNode
{
public:
  BorderEffectNode(VRefCnt* cnt, Ref<RenderNode> child)
    : EffectNode(cnt, std::move(child))
  {
  }
};

class FillEffectNodeImpl : public FillEffectNode
{
public:
  FillEffectNodeImpl(VRefCnt* cnt, Ref<RenderNode> child, const Fill& fill)
    : FillEffectNode(cnt, std::move(child))
  {
  }

protected:
  Bounds onRevalidate() override
  {
    return getChild()->bounds();
  }
};

class BorderEffectNodeImpl : public BorderEffectNode
{
public:
  BorderEffectNodeImpl(VRefCnt* cnt, Ref<RenderNode> child, const Border& border)
    : BorderEffectNode(cnt, std::move(child))
  {
  }
};

class DropShadowEffectNode : public EffectNode
{
public:
  DropShadowEffectNode(VRefCnt* cnt, Ref<RenderNode> child)
    : EffectNode(cnt, std::move(child))
  {
  }
};

class DropShadowEffectNodeImpl : public DropShadowEffectNode
{
public:
  DropShadowEffectNodeImpl(VRefCnt* cnt, Ref<RenderNode> child, const DropShadow& dropShadow)
    : DropShadowEffectNode(cnt, std::move(child))
  {
  }
};

class InnerShadowEffectNode : public EffectNode
{
public:
  InnerShadowEffectNode(VRefCnt* cnt, Ref<RenderNode> child)
    : EffectNode(cnt, std::move(child))
  {
  }
};

class InnerShadowEffectNodeImpl : public InnerShadowEffectNode
{
public:
  InnerShadowEffectNodeImpl(VRefCnt* cnt, Ref<RenderNode> child, const InnerShadow& innerShadow)
    : InnerShadowEffectNode(cnt, std::move(child))
  {
  }
};
} // namespace VGG::layer
