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

#include "Layer/Config.hpp"

#include "Layer/FontManager.hpp"
#include "Layer/Core/Node.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/VType.hpp"

namespace VGG
{

class TextNode__pImpl;
class VGG_EXPORTS TextNode final : public PaintNode
{
  VGG_DECL_IMPL(TextNode);
  friend class NlohmannBuilder;

public:
  TextNode(const std::string& name, std::string guid);
  TextNode(const TextNode&);
  TextNode& operator=(const TextNode&) = delete;
  TextNode(TextNode&&) noexcept = default;
  TextNode& operator=(TextNode&&) noexcept = default;
  void setParagraph(std::string utf8,
                    const std::vector<TextAttr>& attrs,
                    const std::vector<TextLineAttr>& lineAttr);
  void setFrameMode(ETextLayoutMode mode);
  void setVerticalAlignment(ETextVerticalAlignment vertAlign);
  NodePtr clone() const override;
  ~TextNode();

protected:
  void paintEvent(SkiaRenderer* renderer) override;
};

} // namespace VGG
