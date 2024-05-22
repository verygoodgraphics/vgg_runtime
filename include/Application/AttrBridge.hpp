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
struct Color;

namespace layer
{
class PaintNode;
class Accessor;
} // namespace layer

namespace Model
{
struct Object;
struct Color;
} // namespace Model

typedef std::array<double, 6> TDesignMatrix;
typedef glm::mat3             TRenderMatrix;

class AttrBridge
{
  friend ReplaceNodeAnimate;

public:
  AttrBridge(std::shared_ptr<UIView> view, AnimateManage& animateManage);

public:
  bool updateColor(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    const Model::Color&            newColor,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  bool updateFillOpacity(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    double                         newOpacity,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  bool updateOpacity(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    double                         newOpacity,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  bool updateVisible(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    bool                        visible,
    bool                        isOnlyUpdatePaint);

  // Note: The coordinate system that newMatrix resides in depends on the value of isBasedOnGlobal.
  bool updateMatrix(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    const TDesignMatrix&           newMatrix,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {},
    bool                           isNotScaleButChangeSize = false,
    bool                           isFaker = false,
    bool                           isBasedOnGlobal = false);

  bool updateSize(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    double                         newWidth,
    double                         newHeight,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  // Note: if createNewPaintNode is true
  //  1. oldNode already removed and newNode already added.
  //  2. newPaintNode must be nullptr.
  //  3. the type of newNode is frame or symbol-instance.
  bool replaceNode(
    const std::shared_ptr<LayoutNode>   oldNode,
    const std::shared_ptr<LayoutNode>   newNode,
    layer::PaintNode*                   oldPaintNode,
    layer::PaintNode*                   newPaintNode,
    bool                                isOnlyUpdatePaint,
    std::shared_ptr<ReplaceNodeAnimate> animate = {},
    bool                                createNewPaintNode = false);

public:
  layer::PaintNode* getPaintNode(std::shared_ptr<LayoutNode> node);

public:
  static std::optional<VGG::Color>    getFillColor(layer::PaintNode* node, size_t index);
  static std::optional<double>        getFillOpacity(layer::PaintNode* node, size_t index);
  static std::optional<size_t>        getFillSize(layer::PaintNode* node);
  static std::optional<double>        getOpacity(layer::PaintNode* node);
  static std::optional<bool>          getVisible(layer::PaintNode* node);
  static std::optional<TDesignMatrix> getMatrix(layer::PaintNode* node);
  static std::optional<TDesignMatrix> getGlobalMatrix(layer::PaintNode* node);
  static std::optional<double>        getWidth(layer::PaintNode* node);
  static std::optional<double>        getHeight(layer::PaintNode* node);

private:
  static void setFillColor(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    const std::vector<double>&  argb);
  static void setFillColor(layer::PaintNode* node, size_t index, const std::vector<double>& argb);

  static void setFillOpacity(std::shared_ptr<LayoutNode> node, size_t index, double value);
  static void setFillOpacity(layer::PaintNode* node, size_t index, double value);

  static void setOpacity(std::shared_ptr<LayoutNode> node, double value);
  static void setOpacity(layer::PaintNode* node, double value);

  static void setVisible(std::shared_ptr<LayoutNode> node, bool value);
  static void setVisible(layer::PaintNode* node, bool value);

  static void setMatrix(std::shared_ptr<LayoutNode> node, const TDesignMatrix& designMatrix);
  static void setMatrix(layer::PaintNode* node, const TDesignMatrix& designMatrix);

  static void setWidth(std::shared_ptr<LayoutNode> node, const double width);
  static void setWidth(layer::PaintNode* node, const double width);

  static void setHeight(std::shared_ptr<LayoutNode> node, const double height);
  static void setHeight(layer::PaintNode* node, const double height);

  // TODO just like updateMatrix, should accept isNotScaleButChangeSize
  static void setTwinMatrix(
    std::shared_ptr<LayoutNode> nodeFrom,
    std::shared_ptr<LayoutNode> nodeTo,
    layer::PaintNode*           paintNodeTo,
    double                      originalWidthFrom,
    double                      originalHeightFrom,
    double                      originWidthTo,
    double                      originHeightTo,
    const std::vector<double>&  value,
    bool                        isOnlyUpdatePaint);

private:
  void updateSimpleAttr(
    std::shared_ptr<LayoutNode>                     node,
    const std::vector<double>&                      from,
    const std::vector<double>&                      to,
    std::function<void(const std::vector<double>&)> update,
    std::shared_ptr<NumberAnimate>                  animate);

  static Model::Object* getlayoutNodeObject(std::shared_ptr<LayoutNode> node);

private:
  std::shared_ptr<UIView> m_view;
  AnimateManage&          m_animateManage;
};

class TransformHelper
{
  friend AttrBridge;

public:
  static TDesignMatrix transform(
    double               selfWidth,
    double               selfHeight,
    double               desWidth,
    double               desHeight,
    const TDesignMatrix& desMatrix);

  // Note: When the matrix of the top-level frame is [1, 0, 0, 1, 0, 0], it is positioned at the top
  // left corner of the window.
  static TDesignMatrix moveToWindowTopLeft(
    double               width,
    double               height,
    const TDesignMatrix& matrix);

  // base on the design coordinate system.
  static TDesignMatrix translate(double x, double y, const TDesignMatrix& matrix);

  // return [ left-x, top-y, right-x, bottom-y ], base on the design coordinate system.
  static std::array<double, 4> getLTRB(double width, double height, const TDesignMatrix& matrix);

  // base on the render coordinate system.
  static std::array<double, 2> getTranslate(const TRenderMatrix& renderMatrix);
  static std::array<double, 2> getScale(const TRenderMatrix& renderMatrix);
  static double                getRotate(const TRenderMatrix& renderMatrix);

  static TRenderMatrix createRenderMatrix(
    const std::array<double, 2>& translate,
    const std::array<double, 2>& scale,
    double                       rotate);

  static TRenderMatrix fromDesignMatrix(const TDesignMatrix& matrix);
  static TDesignMatrix toDesignMatrix(const TRenderMatrix& transform);

private:
  static void changeYDirection(glm::mat3& transform);

  // base on the design coordinate system.
  static std::pair<double, double> calcXY(double x, double y, const TDesignMatrix& matrix);
};

} // namespace VGG