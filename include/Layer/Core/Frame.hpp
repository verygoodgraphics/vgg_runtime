/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Layer/Core/VBound.hpp"
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
#ifdef USE_SHARED_PTR
using FramePtr = std::shared_ptr<Frame>;
using FrameRef = std::weak_ptr<Frame>;
#else
using FramePtr = VGG::layer::Ref<Frame>;
using FrameRef = VGG::layer::WeakRef<Frame>;
#endif

template<typename... Args>
inline FramePtr makeFramePtr(Args&&... args)
{
#ifdef USE_SHARED_PTR
  auto p = std::make_shared<Frame>(nullptr, std::forward<Args>(args)...);
  return p;
#else
  return FramePtr(V_NEW<Frame>(std::forward<Args>(args)...));
#endif
};
class Frame__pImpl;
class Frame final : public VNode
{
  friend class VGG::Scene;
  friend class VGG::Scene__pImpl;
  VGG_DECL_IMPL(Frame);

public:
  Frame(VRefCnt* cnt, PaintNodePtr root);
  const std::string& guid() const;
  PaintNode*         root() const;

  PaintNode* nodeAt(int x, int y);
  void       nodeAt(int x, int y, std::vector<PaintNode*>& nodes);
  PaintNode* nodeByID(const std::string& id);

  void             resetToOrigin(bool enable);
  bool             isVisible() const;
  const Transform& transform() const;
  void             setClipBound(const Bound& bound);

  ~Frame();

protected:
  Bound      onRevalidate() override;
  SkPicture* picture();
};
} // namespace VGG::layer
