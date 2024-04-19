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
#include "Layer/VGGLayer.hpp"
#include "Layer/Core/Frame.hpp"
#include "Layer/Config.hpp"

namespace VGG::layer
{
class SceneNode__pImpl;
class VGG_EXPORTS SceneNode : public RenderNode
{
  VGG_DECL_IMPL(SceneNode);

public:
  SceneNode(VRefCnt* cnt, std::vector<FramePtr> frames);

  const std::vector<FramePtr>& frames() const;

  void       setFrames(std::vector<FramePtr> frames);
  void       insertFrame(int index, FramePtr frame);
  Frame*     frame(int index);
  void       eraseFrame(int index);
  void       render(Renderer* canvas) override;
  void       nodeAt(int x, int y, layer::PaintNode::NodeVisitor visitor);
  Bounds     effectBounds() const override;
  Bounds     onRevalidate() override;
  glm::mat3  getMatrix() const;
  SkPicture* picture() const override;

  VGG_CLASS_MAKE(SceneNode);
  virtual ~SceneNode();
};

}; // namespace VGG::layer
