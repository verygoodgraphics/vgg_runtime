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
class Frame;
class Renderer;
using FramePtr = VGG::layer::Ref<Frame>;
using FrameRef = VGG::layer::WeakRef<Frame>;

template<typename... Args>
inline FramePtr makeFramePtr(Args&&... args)
{
  return FramePtr(V_NEW<Frame>(std::forward<Args>(args)...));
};
class Frame__pImpl;
class Frame final : public VNode
{
  friend class VGG::Scene;
  friend class VGG::Scene__pImpl;
  friend class SceneNode__pImpl;
  VGG_DECL_IMPL(Frame);

public:
  Frame(VRefCnt* cnt, PaintNodePtr root);
  const std::string& guid() const;
  PaintNode*         node() const;

  void       nodeAt(int x, int y, PaintNode::NodeVisitor visitor);
  PaintNode* nodeByID(const std::string& id);

  void invalidateMask(); // temporary solution

  void             resetToOrigin(bool enable);
  bool             isVisible() const;
  const Transform& transform() const;
  void             setClipBounds(const Bounds& bounds);

  ~Frame();

protected:
  Bounds     onRevalidate() override;
  SkPicture* picture();
};
} // namespace VGG::layer
