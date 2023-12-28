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
#include "Layer/Core/VType.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Memory/VRefCnt.hpp"
#include "Layer/ParagraphLayout.hpp"
#include <utility>

namespace VGG::layer
{

class TextNode;
#ifdef USE_SHARED_PTR
using TextNodePtr = std::shared_ptr<TextNode>;
#else
using TextNodePtr = Ref<TextNode>;
#endif

template<typename... Args>
inline TextNodePtr makeTextNodePtr(Args&&... args)
{
#ifdef USE_SHARED_PTR
  auto p = std::make_shared<TextNode>(nullptr, std::forward<Args>(args)...);
  return p;
#else
  return TextNodePtr(V_NEW<TextNode>(std::forward<Args>(args)...));
#endif
}

class TextNode__pImpl;
class VGG_EXPORTS TextNode final : public PaintNode
{
  VGG_DECL_IMPL(TextNode);

public:
  TextNode(VRefCnt* cnt, const std::string& name, std::string guid);
  TextNode(const TextNode&) = delete;
  TextNode& operator=(const TextNode&) = delete;
  TextNode(TextNode&&) noexcept = delete;
  TextNode& operator=(TextNode&&) noexcept = delete;

  void setParagraph(
    std::string                      utf8,
    std::vector<TextStyleAttr>       attrs,
    const std::vector<TextLineAttr>& lineAttr);
  void setFrameMode(ETextLayoutMode mode);
  void setVerticalAlignment(ETextVerticalAlignment vertAlign);

  ~TextNode();

protected:
  void paintFill(Renderer* renderer, sk_sp<SkBlender> blender, const SkPath& path) override;

  Bound onRevalidate() override;
};

} // namespace VGG::layer
