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
#include <optional>
#include <memory>
#include <string>

namespace VGG
{

class LayoutNode;
class Animate;
class NumberAnimate;
class AnimateManage;

namespace layer
{
class VLayer;
class PaintNode;
} // namespace layer

namespace Model
{
struct Color;
}

class AttrBridge
{
public:
  AttrBridge(layer::VLayer* vLayer, AnimateManage& animateManage);

public:
  bool updateColor(
    std::shared_ptr<LayoutNode>    node,
    size_t                         index,
    const Model::Color&            newColor,
    std::shared_ptr<NumberAnimate> animate = {});

  //   void replaceNode(
  //     const LayoutNode* oldNode,
  //     const LayoutNode* newNode,
  //     const Animate*    animate = nullptr);

private:
  layer::PaintNode* nodeAt(const std::string& id);

private:
  layer::VLayer* m_layer;
  AnimateManage& m_animateManage;
};

} // namespace VGG