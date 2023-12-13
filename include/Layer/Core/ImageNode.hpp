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
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Config.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Renderer.hpp"
#include <core/SkBlendMode.h>

class SkImage;
class SkCanvas;

namespace VGG::layer
{
class ImageNode__pImpl;
class VGG_EXPORTS ImageNode final : public PaintNode
{
  VGG_DECL_IMPL(ImageNode)
public:
  ImageNode(const std::string& name, std::string guid);
  ImageNode(const ImageNode&);
  ImageNode& operator=(const ImageNode&) = delete;

  ImageNode(ImageNode&&) noexcept = delete;
  ImageNode&         operator=(ImageNode&&) noexcept = delete;
  // void paintEvent(SkCanvas* canvas) override;
  void               setImage(const std::string& guid);
  const std::string& getImageGUID() const;
  void               setReplacesImage(bool fill);
  bool               fill() const;
  Mask               asOutlineMask(const Transform* mat) override;

  virtual ~ImageNode() override;

protected:
  void paintFill(Renderer* renderer, sk_sp<SkBlender> blender, const SkPath& path) override;
};
} // namespace VGG::layer
