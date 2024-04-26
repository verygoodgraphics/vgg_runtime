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
#include <array>
#include <functional>

namespace VGG
{

class LayoutNode;
class Animate;
class NumberAnimate;
class ReplaceNodeAnimate;
class AnimateManage;

namespace layer
{
class VLayer;
class PaintNode;
class Transform;
} // namespace layer

namespace Model
{
struct Object;
struct Color;
} // namespace Model

class AttrBridge : public std::enable_shared_from_this<AttrBridge>
{
public:
  AttrBridge(layer::VLayer* vLayer, AnimateManage& animateManage);

public:
  bool updateColor(
    std::shared_ptr<LayoutNode>    node,
    size_t                         index,
    const Model::Color&            newColor,
    bool                           isOnlyUpdatePaint = false,
    std::shared_ptr<NumberAnimate> animate = {});

  bool updateOpacity(
    std::shared_ptr<LayoutNode>    node,
    double                         newOpacity,
    bool                           isOnlyUpdatePaint = false,
    std::shared_ptr<NumberAnimate> animate = {});

  bool updateMatrix(
    std::shared_ptr<LayoutNode>    node,
    const std::array<double, 6>&   newMatrix,
    bool                           isOnlyUpdatePaint = false,
    std::shared_ptr<NumberAnimate> animate = {});

  bool replaceNode(
    const std::shared_ptr<LayoutNode>   oldNode,
    const std::shared_ptr<LayoutNode>   newNode,
    bool                                correlateById,
    bool                                isOnlyUpdatePaint = false,
    std::shared_ptr<ReplaceNodeAnimate> animate = {});

public:
  // std::optional<Model::Color> getNodeColor(std::shared_ptr<LayoutNode> node, bool forPaintNode);
  std::optional<double> getNodeOpacity(std::shared_ptr<LayoutNode> node, bool forPaintNode);
  std::optional<std::array<double, 6>> getNodeMatrix(
    std::shared_ptr<LayoutNode> node,
    bool                        forPaintNode);

private:
  // void setNodeColor(std::shared_ptr<LayoutNode> node, const Model::Color& value, bool
  // forPaintNode);
  void setNodeOpacity(std::shared_ptr<LayoutNode> node, double value, bool forPaintNode);
  void setNodeMatrix(
    std::shared_ptr<LayoutNode>  node,
    const std::array<double, 6>& value,
    bool                         forPaintNode);

private:
  bool updateSimpleAttr(
    std::shared_ptr<LayoutNode>                     node,
    layer::PaintNode*                               paintNode,
    const std::vector<double>&                      from,
    const std::vector<double>&                      to,
    std::function<void(const std::vector<double>&)> updateFrom,
    std::function<void(const std::vector<double>&)> updateTo,
    std::shared_ptr<NumberAnimate>                  animate);

  static Model::Object* getlayoutNodeObject(std::shared_ptr<LayoutNode> node);
  layer::PaintNode*     getPaintNode(std::shared_ptr<LayoutNode> node);

private:
  layer::VLayer* m_layer;
  AnimateManage& m_animateManage;
};

class TransformHelper
{
public:
  typedef std::array<double, 6> TVggMatrix;

public:
  static layer::Transform fromVggMatrix(const TVggMatrix& matrix);
  static TVggMatrix       toVggMatrix(layer::Transform transform);

private:
  static void changeYDirection(layer::Transform& transform);
};

} // namespace VGG