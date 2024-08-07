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
#include <variant>
#include <Math/Algebra.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/glm.hpp>

namespace VGG
{

class UIView;
class LayoutNode;
class Animate;
class NumberAnimate;
class ReplaceNodeAnimate;
class AnimateManage;
struct ImageFilter;
// enum EBlendMode;

namespace layer
{
class PaintNode;
class Accessor;
} // namespace layer

namespace Model
{
struct Object;
struct Fill;
struct Color;
struct ImageFilters;
struct GradientStop;
} // namespace Model

typedef std::array<double, 6> TDesignMatrix;
typedef glm::mat3             TRenderMatrix;

// Note: when animate is running, caller must hold AttrBridge.
class AttrBridge
{
  friend ReplaceNodeAnimate;

public:
  AttrBridge(std::shared_ptr<UIView> view, AnimateManage& animateManage);

public:
  // index equal -1 means push back.
  // The updateFillEnabled interface can be used to enhance the animation capabilities.
  bool addFill(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    const VGG::Model::Fill&     value,
    bool                        isOnlyUpdatePaint,
    size_t                      index = -1);

  // index equal -1 means delete back.
  // The updateFillEnabled interface can be used to enhance the animation capabilities.
  bool delFill(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    bool                        isOnlyUpdatePaint,
    size_t                      index = -1);

  // for fill.isEnabled
  bool updateFillEnabled(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    bool                           enabled,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  // for fill.color
  bool updateFillColor(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    const Model::Color&            newColor,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  // for fill.contextSettings.opacity
  bool updateFillOpacity(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    double                         newOpacity,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  // for fill.contextSettings.blendMode
  bool updateFillBlendMode(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    size_t                      index,
    int                         newBlendMode,
    bool                        isOnlyUpdatePaint);

public:
  // for patternImageFill.imageFileName
  // for patternImageStretch.imageFileName
  // for patternImageFit.imageFileName
  // for patternImageTile.imageFileName
  bool updatePatternImageFileName(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    size_t                      index,
    std::string                 newName,
    bool                        isOnlyUpdatePaint,
    bool                        effectOnFill);

  // for patternImageFill.imageFilters
  // for patternImageStretch.imageFilters
  // for patternImageFit.imageFilters
  // for patternImageTile.imageFilters
  bool updatePatternImageFilters(
    std::shared_ptr<LayoutNode>        node,
    layer::PaintNode*                  paintNode,
    size_t                             index,
    std::optional<Model::ImageFilters> imageFilters,
    bool                               isOnlyUpdatePaint,
    bool                               effectOnFill,
    std::shared_ptr<NumberAnimate>     animate = {});

  // for patternImageFill.rotation
  // for patternImageFit.rotation
  // for patternImageTile.rotation
  bool updatePatternImageFillRotation(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    double                         newRotation,
    bool                           isOnlyUpdatePaint,
    bool                           effectOnFill,
    std::shared_ptr<NumberAnimate> animate = {});

  // for patternImageTile.mirror
  bool updatePatternImageTileMirror(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    size_t                      index,
    bool                        newMirror,
    bool                        isOnlyUpdatePaint,
    bool                        effectOnFill);

  // for patternImageTile.scale
  bool updatePatternImageTileScale(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    double                         newScale,
    bool                           isOnlyUpdatePaint,
    bool                           effectOnFill,
    std::shared_ptr<NumberAnimate> animate = {});

  // for patternImageTile.mode
  bool updatePatternImageTileMode(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    size_t                      index,
    int                         newMode,
    bool                        isOnlyUpdatePaint,
    bool                        effectOnFill);

  // TODO patternImageStretch.matrix not complete.

public:
  // for gradient.from or gradient.to
  // gradient includes gradientLinear, gradientRadial, gradientDiamond, gradientAngular
  bool updateGradientFromOrTo(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    double                         newX,
    double                         newY,
    bool                           isOnlyUpdatePaint,
    bool                           forFrom,
    bool                           effectOnFill,
    std::shared_ptr<NumberAnimate> animate = {});

  // for gradient.ellipse
  // gradient includes gradientRadial, gradientDiamond, gradientAngular
  // If the actual type of the newEllipse does not match the actual type of the corresponding
  // ellipse, there will be no animation effect.
  bool updateGradientEllipse(
    std::shared_ptr<LayoutNode>                 node,
    layer::PaintNode*                           paintNode,
    size_t                                      index,
    std::variant<double, std::array<double, 2>> newEllipse,
    bool                                        isOnlyUpdatePaint,
    bool                                        effectOnFill,
    std::shared_ptr<NumberAnimate>              animate = {});

  // for gradient.stops.position
  // gradient includes gradientLinear, gradientRadial, gradientDiamond, gradientAngular
  bool updateGradientStopPosition(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    size_t                         indexForStops,
    double                         newValue,
    bool                           isOnlyUpdatePaint,
    bool                           effectOnFill,
    std::shared_ptr<NumberAnimate> animate = {});

  // for gradient.stops.color
  // gradient includes gradientLinear, gradientRadial, gradientDiamond, gradientAngular
  bool updateGradientStopColor(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    size_t                         index,
    size_t                         indexForStops,
    const VGG::Model::Color&       newColor,
    bool                           isOnlyUpdatePaint,
    bool                           effectOnFill,
    std::shared_ptr<NumberAnimate> animate = {});

  // TODO The render does not implement the functionality of gradientStop.midPoint, so the
  // corresponding update interface has not been added here.

  // for delete gradient.stops
  // gradient includes gradientLinear, gradientRadial, gradientDiamond, gradientAngular
  bool delGradientStop(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    size_t                      index,
    size_t                      indexForStops,
    bool                        isOnlyUpdatePaint,
    bool                        effectOnFill);

  // for add gradient.stops
  // gradient includes gradientLinear, gradientRadial, gradientDiamond, gradientAngular
  bool addGradientStop(
    std::shared_ptr<LayoutNode>     node,
    layer::PaintNode*               paintNode,
    size_t                          index,
    size_t                          indexForStops,
    const VGG::Model::GradientStop& gradientStop,
    bool                            isOnlyUpdatePaint,
    bool                            effectOnFill);

public:
  // for contextSettings.opacity
  bool updateOpacity(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    double                         newOpacity,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

  // for visible
  bool updateVisible(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    bool                        visible,
    bool                        isOnlyUpdatePaint);

  // Note: The coordinate system that newMatrix resides in depends on the value of isBasedOnGlobal.
  // for matrix
  bool updateMatrix(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    const TDesignMatrix&           newMatrix,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {},
    bool                           isNotScaleButChangeSize = false,
    bool                           isFaker = false,
    bool                           isBasedOnGlobal = false);

  // for bounds.width and bounds.height
  bool updateSize(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    double                         newWidth,
    double                         newHeight,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

public:
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

  // Add newNode to containerPaintNode, not change container and newNode.
  // index equal -1 means push back.
  bool addChild(
    const std::shared_ptr<LayoutNode> container,
    const std::shared_ptr<LayoutNode> newNode,
    layer::PaintNode*                 containerPaintNode,
    size_t                            index = -1,
    std::shared_ptr<NumberAnimate>    animate = {});

  // Note: not deal LayoutNode
  bool delChild(
    layer::PaintNode*              paintNode,
    size_t                         index,
    std::shared_ptr<NumberAnimate> animate = {});

  // Note: newValue: [x, y], represents a translate matrix, base on the design coordinate system.
  bool scrollTo(
    std::shared_ptr<LayoutNode>    node,
    layer::PaintNode*              paintNode,
    std::array<double, 2>          newValue,
    bool                           isOnlyUpdatePaint,
    std::shared_ptr<NumberAnimate> animate = {});

public:
  layer::PaintNode*       getPaintNode(std::shared_ptr<LayoutNode> node);
  std::shared_ptr<UIView> getView();
  static Model::Object*   getlayoutNodeObject(std::shared_ptr<LayoutNode> node);

public:
  static std::optional<size_t>      getFillSize(layer::PaintNode* node);
  static std::optional<int>         getFillType(layer::PaintNode* node, size_t index);
  static std::optional<bool>        getFillEnabled(layer::PaintNode* node, size_t index);
  static std::optional<glm::vec4>   getFillColor(layer::PaintNode* node, size_t index);
  static std::optional<double>      getFillOpacity(layer::PaintNode* node, size_t index);
  static std::optional<int>         getFillBlendMode(layer::PaintNode* node, size_t index);
  static std::optional<std::string> getFillPatternType(layer::PaintNode* node, size_t index);

  static std::optional<std::string> getPatternImageFileName(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);
  static std::optional<VGG::ImageFilter> getPatternImageFilters(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);
  static std::optional<double> getPatternRotation(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);
  static std::optional<bool> getPatternImageTileMirror(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);
  static std::optional<double> getPatternImageTileScale(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);
  static std::optional<int> getPatternImageTileMode(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);

  static std::optional<std::array<double, 2>> getGradientFromOrTo(
    layer::PaintNode* node,
    size_t            index,
    bool              forFrom,
    bool              effectOnFill);
  static std::optional<std::variant<double, std::array<double, 2>>> getGradientEllipse(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);
  static std::optional<size_t> getGradientStopsCount(
    layer::PaintNode* node,
    size_t            index,
    bool              effectOnFill);
  static std::optional<double> getGradientStopsPosition(
    layer::PaintNode* node,
    size_t            index,
    size_t            indexForStops,
    bool              effectOnFill);
  static std::optional<glm::vec4> getGradientStopsColor(
    layer::PaintNode* node,
    size_t            index,
    size_t            indexForStops,
    bool              effectOnFill);

  static std::optional<double>                getOpacity(layer::PaintNode* node);
  static std::optional<bool>                  getVisible(layer::PaintNode* node);
  static std::optional<TDesignMatrix>         getMatrix(layer::PaintNode* node);
  static std::optional<TDesignMatrix>         getGlobalMatrix(layer::PaintNode* node);
  static std::optional<double>                getWidth(layer::PaintNode* node);
  static std::optional<double>                getHeight(layer::PaintNode* node);
  static std::optional<size_t>                getChildrenSize(layer::PaintNode* node);
  static std::optional<std::array<double, 2>> getScrollInfo(layer::PaintNode* node);

private:
  static void setFillEnabled(std::shared_ptr<LayoutNode> node, size_t index, bool enabled);
  static void setFillEnabled(layer::PaintNode* node, size_t index, bool enabled);

  static void setFillColor(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    const std::vector<double>&  argb);
  static void setFillColor(layer::PaintNode* node, size_t index, const std::vector<double>& argb);

  static void setFillOpacity(std::shared_ptr<LayoutNode> node, size_t index, double value);
  static void setFillOpacity(layer::PaintNode* node, size_t index, double value);

  static void setFillBlendMode(std::shared_ptr<LayoutNode> node, size_t index, int value);
  static void setFillBlendMode(layer::PaintNode* node, size_t index, const int value);

  static void setPatternImageFileName(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    const std::string&          newName,
    bool                        effectOnFill);
  static void setPatternImageFileName(
    layer::PaintNode*  node,
    size_t             index,
    const std::string& newName,
    bool               effectOnFill);

  static void setPatternImageFilters(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    const std::vector<double>&  value,
    bool                        effectOnFill);
  static void setPatternImageFilters(
    layer::PaintNode*          node,
    size_t                     index,
    const std::vector<double>& value,
    bool                       effectOnFill);

  static void setPatternRotation(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    double                      value,
    bool                        effectOnFill);
  static void setPatternRotation(
    layer::PaintNode* node,
    size_t            index,
    double            value,
    bool              effectOnFill);

  static void setPatternImageTileMirror(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    bool                        value,
    bool                        effectOnFill);
  static void setPatternImageTileMirror(
    layer::PaintNode* node,
    size_t            index,
    bool              value,
    bool              effectOnFill);

  static void setPatternImageTileScale(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    double                      value,
    bool                        effectOnFill);
  static void setPatternImageTileScale(
    layer::PaintNode* node,
    size_t            index,
    double            value,
    bool              effectOnFill);

  static void setPatternImageTileMode(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    int                         value,
    bool                        effectOnFill);
  static void setPatternImageTileMode(
    layer::PaintNode* node,
    size_t            index,
    int               value,
    bool              effectOnFill);

  static void setGradientFromOrTo(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    double                      newX,
    double                      newY,
    bool                        forFrom,
    bool                        effectOnFill);
  static void setGradientFromOrTo(
    layer::PaintNode* node,
    size_t            index,
    double            newX,
    double            newY,
    bool              forFrom,
    bool              effectOnFill);

  static void setGradientEllipse(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    const std::vector<double>&  value,
    bool                        effectOnFill);
  static void setGradientEllipse(
    layer::PaintNode*          node,
    size_t                     index,
    const std::vector<double>& value,
    bool                       effectOnFill);

  static void setGradientStopsPosition(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    size_t                      indexForStops,
    double                      newValue,
    bool                        effectOnFill);
  static void setGradientStopsPosition(
    layer::PaintNode* node,
    size_t            index,
    size_t            indexForStops,
    double            newValue,
    bool              effectOnFill);

  static void setGradientStopsColor(
    std::shared_ptr<LayoutNode> node,
    size_t                      index,
    size_t                      indexForStops,
    const std::vector<double>&  color,
    bool                        effectOnFill);
  static void setGradientStopsColor(
    layer::PaintNode*          node,
    size_t                     index,
    size_t                     indexForStops,
    const std::vector<double>& color,
    bool                       effectOnFill);

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

  static void setTwinMatrix(
    std::shared_ptr<LayoutNode> nodeTo,
    layer::PaintNode*           paintNodeTo,
    double                      originalWidthFrom,
    double                      originalHeightFrom,
    double                      originWidthTo,
    double                      originHeightTo,
    const std::vector<double>&  value,
    bool                        isOnlyUpdatePaint,
    bool                        isNotScaleButChangeSize);

  // x and y base on the design coordinate system.
  static void setContentOffset(std::shared_ptr<LayoutNode> node, const double x, const double y);
  static void setContentOffset(layer::PaintNode* node, const double x, const double y);

private:
  void updateSimpleAttr(
    const std::vector<double>&                      from,
    const std::vector<double>&                      to,
    std::function<void(const std::vector<double>&)> update,
    std::shared_ptr<NumberAnimate>                  animate);

  static bool checkForAccessFill(layer::PaintNode* paintNode, size_t index);

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
  // left corner of the window. boundOriginX and boundOriginY based on the design coordinate system.
  static TDesignMatrix moveToWindowTopLeft(
    double               boundOriginX,
    double               boundOriginY,
    double               width,
    double               height,
    const TDesignMatrix& matrix);

  // base on the design coordinate system.
  static TDesignMatrix translate(double x, double y, const TDesignMatrix& matrix);

  // return [ left-x, top-y, right-x, bottom-y ], base on the design coordinate system.
  static std::array<double, 4> getLTRB(double width, double height, const TDesignMatrix& matrix);

  // base on the render coordinate system.
  // static std::array<double, 2> getTranslate(const TRenderMatrix& renderMatrix);
  // static std::array<double, 2> getScale(const TRenderMatrix& renderMatrix);
  // static double                getRotate(const TRenderMatrix& renderMatrix);

  // base on the render coordinate system.
  // return: [translate_x, translate_y, scale_x, scale_y, rotation]
  static std::array<double, 5> getTSR(const TRenderMatrix& renderMatrix);

  static TRenderMatrix createRenderMatrix(
    const std::array<double, 2>& translate,
    const std::array<double, 2>& scale,
    double                       rotate);

  // base on the render coordinate system.
  static TRenderMatrix createRenderMatrix(
    double width,
    double height,
    double centerXInFatherCoordinateSystem,
    double centerYInFatherCoordinateSystem,
    double scaleX,
    double scaleY,
    double rotate);

  static TRenderMatrix fromDesignMatrix(const TDesignMatrix& matrix);
  static TDesignMatrix toDesignMatrix(const TRenderMatrix& transform);

private:
  static void changeYDirection(glm::mat3& transform);

  // base on the design coordinate system.
  static std::pair<double, double> calcXY(double x, double y, const TDesignMatrix& matrix);
};

} // namespace VGG
