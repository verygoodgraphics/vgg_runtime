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
#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Config.hpp"
#include "Layer/Memory/VNew.hpp"

namespace VGG::layer
{
class ImageNode;
using ImageNodePtr = Ref<ImageNode>;

template<typename... Args>
inline ImageNodePtr makeImageNodePtr(Args&&... args)
{
  return ImageNodePtr(V_NEW<ImageNode>(std::forward<Args>(args)...));
};
class ImageNode__pImpl;
class VGG_EXPORTS ImageNode final : public PaintNode
{
  VGG_DECL_IMPL(ImageNode)
  friend class SceneBuilder;

public:
  ImageNode(VRefCnt* cnt, const std::string& name, std::string guid);
  ImageNode(const ImageNode&) = delete;
  ImageNode& operator=(const ImageNode&) = delete;

  ImageNode(ImageNode&&) noexcept = delete;
  ImageNode& operator=(ImageNode&&) noexcept = delete;

  void               setImageBounds(const Bounds& bounds);
  Bounds             getImageBounds() const;
  void               setImage(const std::string& guid);
  const std::string& getImageGUID() const;
  void               setImageFilter(const ImageFilter& filter);
  VShape             asVisualShape(const Transform* mat) override;

  using EventHandler = std::function<void(ImageItemAttribtueAccessor*, void* event)>;
  void installImageNodeEventHandler(EventHandler handler);

  virtual ~ImageNode() override;

protected:
  void dispatchEvent(void* event) override;
};
} // namespace VGG::layer
