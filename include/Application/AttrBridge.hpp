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
#include <Math/Algebra.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

namespace VGG
{

class UIView;
class LayoutNode;
class Animate;
class NumberAnimate;
class ReplaceNodeAnimate;
class AnimateManage;

namespace layer
{
class PaintNode;
} // namespace layer

namespace Model
{
struct Object;
struct Color;
} // namespace Model

class AttrBridge
{
public:
  AttrBridge(std::shared_ptr<UIView> view, AnimateManage& animateManage);

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

  bool updateVisible(
    std::shared_ptr<LayoutNode> node,
    bool                        visible,
    bool                        isOnlyUpdatePaint = false);

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
  static std::optional<double> getOpacity(std::shared_ptr<LayoutNode> node);

  static std::optional<bool> getVisible(std::shared_ptr<LayoutNode> node);

  static std::optional<std::array<double, 6>> getMatrix(std::shared_ptr<LayoutNode> node);

private:
  static void setOpacity(std::shared_ptr<LayoutNode> node, double value);
  static void setOpacity(layer::PaintNode* node, double value);

  static void setVisible(std::shared_ptr<LayoutNode> node, bool value);
  static void setVisible(layer::PaintNode* node, bool value);

  static void setMatrix(
    std::shared_ptr<LayoutNode>  node,
    const std::array<double, 6>& designMatrix);
  static void setMatrix(layer::PaintNode* node, const std::array<double, 6>& designMatrix);

private:
  void updateSimpleAttr(
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
  std::shared_ptr<UIView> m_view;
  AnimateManage&          m_animateManage;
};

class TransformHelper
{
  friend AttrBridge;

public:
  typedef std::array<double, 6> TDesignMatrix;
  typedef glm::mat3             TRenderMatrix;

public:
  static TDesignMatrix transform(
    double               selfWidth,
    double               selfHeight,
    double               desWidth,
    double               desHeight,
    const TDesignMatrix& desMatrix);

private:
  static TRenderMatrix fromDesignMatrix(const TDesignMatrix& matrix);
  static TDesignMatrix toDesignMatrix(const TRenderMatrix& transform);
  static void          changeYDirection(glm::mat3& transform);
};

} // namespace VGG