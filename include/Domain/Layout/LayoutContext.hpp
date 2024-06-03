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

#include <memory>
#include <optional>
#include "Domain/Layout/LayoutNode.hpp"
#include "Domain/Layout/Rect.hpp"

namespace VGG
{
class LayoutNode;

class LayoutContext
{
public:
  virtual ~LayoutContext() = default;

  virtual bool isLayerValid() const = 0;
  virtual void setLayerValid(bool valid) = 0;

  virtual std::optional<Layout::Size> nodeSize(LayoutNode* node) const = 0;

  virtual void didUpdateBounds(LayoutNode* node) = 0;
  virtual void didUpdateMatrix(LayoutNode* node) = 0;
  virtual void didUpdateContourPoints(LayoutNode* node) = 0;
};

} // namespace VGG