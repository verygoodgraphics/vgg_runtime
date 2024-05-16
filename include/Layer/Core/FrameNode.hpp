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
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/TransformNode.hpp"
#include "Layer/Core/VBounds.hpp"
#include "Utility/HelperMacro.hpp"

class SkPicture;
template<typename T>
class sk_sp;

namespace VGG
{
class Scene__pImpl;
class Scene;
} // namespace VGG
namespace VGG::layer
{
class FrameNode;
class Renderer;
using FramePtr = VGG::layer::Ref<FrameNode>;
using FrameRef = VGG::layer::WeakRef<FrameNode>;

template<typename... Args>
inline FramePtr makeFramePtr(Args&&... args)
{
  return FramePtr(V_NEW<FrameNode>(std::forward<Args>(args)...));
};
class FrameNode__pImpl;
class FrameNode final : public TransformEffectNode
{
  friend class VGG::Scene;
  friend class VGG::Scene__pImpl;
  friend class SceneNode__pImpl;
  VGG_DECL_IMPL(FrameNode);

public:
  FrameNode(VRefCnt* cnt, PaintNodePtr root);
  const std::string& guid() const;
  PaintNode*         node() const;

  Bounds effectBounds() const override;

  void nodeAt(int x, int y, NodeVisitor vistor, void* userData) override;

  PaintNode* nodeByID(int id);

  void invalidateMask(); // temporary solution

  void             resetToOrigin(bool enable);
  bool             isVisible() const;
  const Transform& transform() const;

  void render(Renderer* renderer) override;

#ifdef VGG_LAYER_DEBUG
  void debug(Renderer* render) override;
#endif

  SkPicture* picture() const override;

  ~FrameNode();

protected:
  Bounds onRevalidate() override;
};
} // namespace VGG::layer
