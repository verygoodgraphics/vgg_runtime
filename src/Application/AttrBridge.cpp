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
#include "Application/AttrBridge.hpp"
#include "Application/Animate.hpp"
#include "Application/UIView.hpp"
#include "Domain/Layout/Node.hpp"
#include "Domain/Model/Element.hpp"
#include "Domain/Model/DesignModel.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Memory/Ref.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/SceneBuilder.hpp"

using namespace VGG;

AttrBridge::AttrBridge(std::shared_ptr<UIView> view, AnimateManage& animateManage)
  : m_view(view)
  , m_animateManage(animateManage)
{
}

#define CHECK_EXPR(expr, resultWhenFailed)                                                         \
  if (!expr)                                                                                       \
  {                                                                                                \
    return resultWhenFailed;                                                                       \
  }

#define GET_PAINTNODE_ACCESSOR(node, accessor, resultWhenFailed)                                   \
  CHECK_EXPR(node, resultWhenFailed)                                                               \
  auto accessor = node->attributeAccessor();                                                       \
  CHECK_EXPR(accessor, resultWhenFailed)

std::optional<VGG::Color> AttrBridge::getFillColor(layer::PaintNode* node, size_t index)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});

  try
  {
    return std::get<VGG::Color>(accessor->getFills().at(index).type);
  }
  catch (...)
  {
  }

  return {};
}

std::optional<double> AttrBridge::getFillOpacity(layer::PaintNode* node, size_t index)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});

  try
  {
    return static_cast<double>(accessor->getFills().at(index).contextSettings.opacity);
  }
  catch (...)
  {
  }

  return {};
}

std::optional<size_t> AttrBridge::getFillSize(layer::PaintNode* node)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});
  return accessor->getFills().size();
}

std::optional<double> AttrBridge::getOpacity(layer::PaintNode* node)
{
  CHECK_EXPR(node, {});
  return node->contextSetting().opacity;
}

std::optional<bool> AttrBridge::getVisible(layer::PaintNode* node)
{
  CHECK_EXPR(node, {});
  return node->isVisible();
}

std::optional<TDesignMatrix> AttrBridge::getMatrix(layer::PaintNode* node)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});
  return TransformHelper::toDesignMatrix(accessor->getTransform().matrix());
}

std::optional<TDesignMatrix> AttrBridge::getGlobalMatrix(layer::PaintNode* node)
{
  auto matrix = getMatrix(node);
  if (!matrix)
  {
    assert(false);
    return {};
  }

  auto result = TransformHelper::fromDesignMatrix(*matrix);

  while (true)
  {
    node = node->parent();
    if (!node)
    {
      break;
    }

    matrix = getMatrix(node);
    if (!matrix)
    {
      assert(false);
      return {};
    }
    result = TransformHelper::fromDesignMatrix(*matrix) * result;
  }

  return TransformHelper::toDesignMatrix(result);
}

std::optional<double> AttrBridge::getWidth(layer::PaintNode* node)
{
  CHECK_EXPR(node, {});
  return node->frameBounds().width();
}

std::optional<double> AttrBridge::getHeight(layer::PaintNode* node)
{
  CHECK_EXPR(node, {});
  return node->frameBounds().height();
}

void AttrBridge::setFillColor(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  const std::vector<double>&  argb)
{
  auto object = AttrBridge::getlayoutNodeObject(node);
  if (!object || index >= object->style.fills.size())
  {
    return;
  }

  auto& color = object->style.fills.at(index).color;
  color->alpha = static_cast<float>(argb.at(0));
  color->red = static_cast<float>(argb.at(1));
  color->green = static_cast<float>(argb.at(2));
  color->blue = static_cast<float>(argb.at(3));
}

void AttrBridge::setFillColor(layer::PaintNode* node, size_t index, const std::vector<double>& argb)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, void());

  auto fills = accessor->getFills();
  if (index >= fills.size())
  {
    return;
  }

  VGG::Color color;
  color.a = static_cast<float>(argb.at(0));
  color.r = static_cast<float>(argb.at(1));
  color.g = static_cast<float>(argb.at(2));
  color.b = static_cast<float>(argb.at(3));

  fills.at(index).type = color;
  accessor->setFills(fills);
}

void AttrBridge::setFillOpacity(std::shared_ptr<LayoutNode> node, size_t index, double value)
{
  if (auto object = AttrBridge::getlayoutNodeObject(node))
  {
    if (index >= object->style.fills.size())
    {
      return;
    }
    object->style.fills.at(index).contextSettings.opacity = value;
  }
}

void AttrBridge::setFillOpacity(layer::PaintNode* node, size_t index, double value)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, void());

  auto fills = accessor->getFills();
  if (index >= fills.size())
  {
    return;
  }

  fills.at(index).contextSettings.opacity = value;
  accessor->setFills(fills);
}

void AttrBridge::setOpacity(std::shared_ptr<LayoutNode> node, double value)
{
  if (auto object = AttrBridge::getlayoutNodeObject(node))
  {
    object->contextSettings.opacity = value;
  }
}

void AttrBridge::setOpacity(layer::PaintNode* node, double value)
{
  if (node)
  {
    auto contextSetting = node->contextSetting();
    contextSetting.opacity = static_cast<float>(value);
    node->setContextSettings(contextSetting);
  }
}

void AttrBridge::setVisible(std::shared_ptr<LayoutNode> node, bool value)
{
  if (auto object = AttrBridge::getlayoutNodeObject(node))
  {
    object->visible = value;
  }
}

void AttrBridge::setVisible(layer::PaintNode* node, bool value)
{
  if (node)
  {
    node->setVisible(value);
  }
}

void AttrBridge::setMatrix(std::shared_ptr<LayoutNode> node, const TDesignMatrix& designMatrix)
{
  auto object = AttrBridge::getlayoutNodeObject(node);
  if (!object)
  {
    return;
  }

  if (object->matrix.size() != 6)
  {
    assert(false);
    object->matrix.resize(6);
  }

  std::copy(designMatrix.begin(), designMatrix.end(), object->matrix.begin());
}

void AttrBridge::setMatrix(layer::PaintNode* node, const TDesignMatrix& designMatrix)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, void());
  accessor->setTransform(VGG::layer::Transform(TransformHelper::fromDesignMatrix(designMatrix)));
}

void AttrBridge::setWidth(std::shared_ptr<LayoutNode> node, const double width)
{
  auto object = AttrBridge::getlayoutNodeObject(node);
  if (!object)
  {
    return;
  }

  object->bounds.width = width;
}

void AttrBridge::setWidth(layer::PaintNode* node, const double width)
{
  if (node)
  {
    auto bounds = node->frameBounds();
    bounds.setWidth(width);
    node->setFrameBounds(bounds);
  }
}

void AttrBridge::setHeight(std::shared_ptr<LayoutNode> node, const double height)
{
  auto object = AttrBridge::getlayoutNodeObject(node);
  if (!object)
  {
    return;
  }

  object->bounds.height = height;
}

void AttrBridge::setHeight(layer::PaintNode* node, const double height)
{
  if (node)
  {
    auto bounds = node->frameBounds();
    bounds.setHeight(height);
    node->setFrameBounds(bounds);
  }
}

// TODO do not need node
void AttrBridge::updateSimpleAttr(
  std::shared_ptr<LayoutNode>                     node,
  const std::vector<double>&                      from,
  const std::vector<double>&                      to,
  std::function<void(const std::vector<double>&)> update,
  std::shared_ptr<NumberAnimate>                  animate)
{
  if (!animate)
  {
    update(to);
  }
  else
  {
    animate->setFromTo(from, to);
    animate->setAction([update](const std::vector<double>& value) { update(value); });
    animate->start();
    m_animateManage.addAnimate(animate);
  }
}

Model::Object* AttrBridge::getlayoutNodeObject(std::shared_ptr<LayoutNode> node)
{
  if (!node)
  {
    return nullptr;
  }

  auto element = node->elementNode();
  if (!element)
  {
    return nullptr;
  }

  return element->object();
}

layer::PaintNode* AttrBridge::getPaintNode(std::shared_ptr<LayoutNode> node)
{
  if (!node)
  {
    return nullptr;
  }

  auto element = node->elementNode();
  if (!element)
  {
    return nullptr;
  }

  if (!m_view)
  {
    return nullptr;
  }

  const auto& id = element->idNumber();

  // for top-level frame
  if (node->parent() && !node->parent()->parent())
  {
    for (auto& frame : m_view->getSceneNode()->getFrames())
    {
      ASSERT(frame->node());
      if (frame->node()->uniqueID() == id)
      {
        return frame->node();
      }
    }
  }

  return m_view->getSceneNode()->nodeByID(id);
}

bool AttrBridge::updateColor(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  const Model::Color&            newColor,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  GET_PAINTNODE_ACCESSOR(paintNode, accessor, false);

  auto update = [node, paintNode, index, isOnlyUpdatePaint](const std::vector<double>& value)
  {
    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setFillColor(node, index, value);
    }

    AttrBridge::setFillColor(paintNode, index, value);
  };

  VGG::Color color;
  try
  {
    color = std::get<VGG::Color>(accessor->getFills().at(index).type);
  }
  catch (...)
  {
    return false;
  }

  updateSimpleAttr(
    node,
    { color.a, color.r, color.g, color.b },
    { newColor.alpha, newColor.red, newColor.green, newColor.blue },
    update,
    animate);

  return true;
}

bool AttrBridge::updateFillOpacity(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  double                         newOpacity,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  if (!paintNode)
  {
    return false;
  }

  auto update = [node, paintNode, index, isOnlyUpdatePaint](const std::vector<double>& value)
  {
    assert(value.size() == 1);

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setFillOpacity(node, index, value.at(0));
    }

    AttrBridge::setFillOpacity(paintNode, index, value.at(0));
  };

  auto nowValue = AttrBridge::getFillOpacity(paintNode, index);
  if (!nowValue)
  {
    return false;
  }

  updateSimpleAttr(node, { *nowValue }, { newOpacity }, update, animate);
  return true;
}

bool AttrBridge::updateOpacity(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  double                         newOpacity,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  assert(newOpacity >= 0 && newOpacity <= 1);

  if (!paintNode)
  {
    return false;
  }

  auto update = [node, paintNode, isOnlyUpdatePaint](const std::vector<double>& value)
  {
    assert(value.size() == 1);

    DEBUG(
      "AttrBridge::updateOpacity: node[%s], paintNode[%p] value.at(0) = %f",
      node->id().c_str(),
      paintNode,
      value.at(0));

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setOpacity(node, value.at(0));
    }

    AttrBridge::setOpacity(paintNode, value.at(0));
  };

  updateSimpleAttr(node, { *AttrBridge::getOpacity(paintNode) }, { newOpacity }, update, animate);

  return true;
}

bool AttrBridge::updateVisible(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  bool                        visible,
  bool                        isOnlyUpdatePaint)
{
  if (!isOnlyUpdatePaint)
  {
    AttrBridge::setVisible(node, visible);
  }

  AttrBridge::setVisible(paintNode, visible);

  return true;
}

bool AttrBridge::updateMatrix(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  const TDesignMatrix&           newMatrix,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate,
  bool                           isNotScaleButChangeSize,
  bool                           isFaker,
  bool                           isBasedOnGlobal)
{
  if (!paintNode)
  {
    return false;
  }

  std::optional<TDesignMatrix> oldMatrix;
  if (isBasedOnGlobal)
  {
    oldMatrix = AttrBridge::getGlobalMatrix(paintNode);
  }
  else
  {
    oldMatrix = AttrBridge::getMatrix(paintNode);
  }

  if (!oldMatrix)
  {
    assert(false);
    return false;
  }

  auto originWidth = AttrBridge::getWidth(paintNode);
  auto originHeight = AttrBridge::getHeight(paintNode);
  if (!originWidth || !originHeight)
  {
    assert(false);
    return false;
  }

  auto update = [isFaker,
                 node,
                 paintNode,
                 isOnlyUpdatePaint,
                 isNotScaleButChangeSize,
                 isBasedOnGlobal,
                 originWidth,
                 originHeight](const std::vector<double>& value)
  {
    if (isFaker)
    {
      return;
    }

    assert(value.size() == 5);

    std::array<double, 2> scale{ value[2], value[3] };

    // TODO frame can not scale, should change width and height, what about group and symbol?
    if (isNotScaleButChangeSize)
    {
      auto newWidth = scale[0] * (*originWidth);
      auto newHeight = scale[1] * (*originHeight);
      AttrBridge::setWidth(paintNode, newWidth);
      AttrBridge::setHeight(paintNode, newHeight);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setWidth(node, newWidth);
        AttrBridge::setHeight(node, newHeight);
      }

      scale = { 1.0, 1.0 };
    }

    auto matrix =
      TransformHelper::createRenderMatrix({ value[0], value[1] }, { scale[0], scale[1] }, value[4]);
    TDesignMatrix finalMatrix{};

    if (isBasedOnGlobal)
    {
      glm::mat3 invMatrixs{ 1.0 };
      auto      parent = paintNode->parent();

      while (parent)
      {
        auto parentMatrix = getMatrix(parent);
        if (!parentMatrix)
        {
          assert(false);
          return;
        }

        invMatrixs *= layer::Transform(TransformHelper::fromDesignMatrix(*parentMatrix)).inverse();
        parent = parent->parent();
      }

      finalMatrix = TransformHelper::toDesignMatrix(layer::Transform(invMatrixs * matrix).matrix());
    }
    else
    {
      finalMatrix = TransformHelper::toDesignMatrix(matrix);
    }

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setMatrix(node, finalMatrix);
    }

    AttrBridge::setMatrix(paintNode, finalMatrix);
  };

  // TODO layer Transform check, should use in last.
  auto getInfo = [](const TRenderMatrix& transform)
  {
    auto translate = TransformHelper::getTranslate(transform);
    auto scale = TransformHelper::getScale(transform);
    auto rotate = TransformHelper::getRotate(transform);
    return std::vector<double>{ translate[0], translate[1], scale[0], scale[1], rotate };
  };

  std::vector<double> from = getInfo(TransformHelper::fromDesignMatrix(*oldMatrix));
  std::vector<double> to = getInfo(TransformHelper::fromDesignMatrix(newMatrix));
  updateSimpleAttr(node, from, to, update, animate);

  if (animate)
  {
    animate->addCallBackWhenStop(
      [node, paintNode, originWidth, originHeight, isOnlyUpdatePaint]()
      {
        AttrBridge::setWidth(paintNode, *originWidth);
        AttrBridge::setHeight(paintNode, *originHeight);

        if (!isOnlyUpdatePaint)
        {
          AttrBridge::setWidth(node, *originWidth);
          AttrBridge::setHeight(node, *originHeight);
        }
      });
  }

  return true;
}

void AttrBridge::setTwinMatrix(
  std::shared_ptr<LayoutNode> nodeFrom,
  std::shared_ptr<LayoutNode> nodeTo,
  layer::PaintNode*           paintNodeTo,
  double                      originalWidthFrom,
  double                      originalHeightFrom,
  double                      originWidthTo,
  double                      originHeightTo,
  const std::vector<double>&  value,
  bool                        isOnlyUpdatePaint)
{
  if (!nodeFrom || !nodeTo || !paintNodeTo)
  {
    assert(false);
    return;
  }

  glm::vec2 translate{ static_cast<float>(value.at(0)), static_cast<float>(value.at(1)) };
  glm::vec2 scale{ static_cast<float>(value.at(2)), static_cast<float>(value.at(3)) };
  auto      rotate = static_cast<float>(value.at(4));

  double width = originalWidthFrom * scale[0];
  double height = originalHeightFrom * scale[1];

  if (originWidthTo)
  {
    scale[0] = width / originWidthTo;
  }
  if (originHeightTo)
  {
    scale[1] = height / originHeightTo;
  }

  if (ReplaceNodeAnimate::isContainerType(nodeFrom->elementNode()))
  {
    // TODO change nodeTo size

    auto newWidth = scale[0] * originWidthTo;
    auto newHeight = scale[1] * originHeightTo;

    AttrBridge::setWidth(paintNodeTo, newWidth);
    AttrBridge::setHeight(paintNodeTo, newHeight);

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setWidth(nodeTo, newWidth);
      AttrBridge::setHeight(nodeTo, newHeight);
    }

    scale = glm::vec2{ 1.0f, 1.0f };
  }

  // TODO can be better
  auto matrix = glm::identity<glm::mat3>();
  matrix = glm::translate(matrix, translate);
  matrix = glm::rotate(matrix, rotate);
  matrix = glm::scale(matrix, scale);

  auto designMatrix = TransformHelper::toDesignMatrix(matrix);

  if (!isOnlyUpdatePaint)
  {
    AttrBridge::setMatrix(nodeTo, designMatrix);
  }

  AttrBridge::setMatrix(paintNodeTo, designMatrix);
}

bool AttrBridge::updateSize(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  double                         newWidth,
  double                         newHeight,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  if (!paintNode || newWidth < 0 || newHeight < 0 || (!isOnlyUpdatePaint && !node))
  {
    assert(false);
    return false;
  }

  double oldWidth = *AttrBridge::getWidth(paintNode);
  double oldHeight = *AttrBridge::getHeight(paintNode);

  auto update = [paintNode, node, isOnlyUpdatePaint](const std::vector<double>& value)
  {
    assert(value.size() == 2);
    AttrBridge::setWidth(paintNode, value[0]);
    AttrBridge::setHeight(paintNode, value[1]);

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setWidth(node, value[0]);
      AttrBridge::setHeight(node, value[1]);
    }
  };

  updateSimpleAttr(node, { oldWidth, oldHeight }, { newWidth, newHeight }, update, animate);
  return true;
}

bool AttrBridge::replaceNode(
  const std::shared_ptr<LayoutNode>   oldNode,
  const std::shared_ptr<LayoutNode>   newNode,
  layer::PaintNode*                   oldPaintNode,
  layer::PaintNode*                   newPaintNode,
  bool                                isOnlyUpdatePaint,
  std::shared_ptr<ReplaceNodeAnimate> animate,
  bool                                createNewPaintNode)
{
#ifdef DEBUG
  if (createNewPaintNode)
  {
    assert(!newPaintNode);

    if (oldNode)
    {
      assert(!oldNode->parent());
    }

    if (newNode)
    {
      assert(newNode->parent());
    }
  }
#endif

  auto removeOldPaintNodeIfNeed = [oldPaintNode, createNewPaintNode]()
  {
    if (createNewPaintNode)
    {
      auto parent = oldPaintNode->parent();
      if (parent)
      {
        parent->removeChild(layer::incRef(oldPaintNode));
      }
    }
  };

  if (createNewPaintNode && !newPaintNode)
  {
    if (!newNode || !oldPaintNode)
    {
      assert(false);
      return false;
    }

    auto element = newNode->elementNode();
    if (!element)
    {
      return false;
    }

    auto                      type = newNode->elementNode()->type();
    layer::SceneBuilderResult result;

    if (type == VGG::Domain::Element::EType::FRAME)
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelFrame>(
        { layer::StructFrameObject(element) });
    }
    else if (type == VGG::Domain::Element::EType::SYMBOL_INSTANCE)
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelInstance>(
        { layer::StructInstanceObject(element) });
    }
    else
    {
      assert(false);
      return false;
    }

    if (!result.root || result.root->size() != 1)
    {
      return false;
    }

    newPaintNode = result.root->at(0)->node();
    if (!newPaintNode)
    {
      return false;
    }

    auto parent = oldPaintNode->parent();
    if (!parent)
    {
      return false;
    }

    auto& children = parent->children();
    auto  it = std::find_if(
      children.begin(),
      children.end(),
      [oldPaintNode](auto& item) { return item == oldPaintNode; });
    if (it == children.end())
    {
      return false;
    }

    parent->addChild(it, layer::incRef(result.root->at(0)->node()));

    if (animate)
    {
      animate->addCallBackWhenStop(removeOldPaintNodeIfNeed);
    }
  }

  if (!animate)
  {
    AttrBridge::setVisible(oldPaintNode, false);
    AttrBridge::setVisible(newPaintNode, true);
    removeOldPaintNodeIfNeed();

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setVisible(oldNode, false);
      AttrBridge::setVisible(newNode, true);
    }
  }
  else
  {
    animate->setFromTo(oldNode, newNode);
    animate->setIsOnlyUpdatePaint(isOnlyUpdatePaint);
    animate->start();
    m_animateManage.addAnimate(animate);
  }
  return true;
}

void TransformHelper::changeYDirection(glm::mat3& transform)
{
  glm::mat3 scale = glm::identity<glm::mat3>();
  scale = glm::scale(scale, { 1, -1 });
  transform = scale * transform * scale;
}

TDesignMatrix TransformHelper::transform(
  double               selfWidth,
  double               selfHeight,
  double               desWidth,
  double               desHeight,
  const TDesignMatrix& desMatrix)
{
  auto renderMatrix = VGG::layer::Transform(TransformHelper::fromDesignMatrix(desMatrix));
  auto offset = renderMatrix.translate();
  auto rotate = renderMatrix.rotate();
  auto scale = renderMatrix.scale();

  auto finalScaleX = 1.0;
  if (selfWidth)
  {
    finalScaleX = desWidth * scale[0] / selfWidth;
  }

  auto finalScaleY = 1.0;
  if (selfHeight)
  {
    finalScaleY = desHeight * scale[1] / selfHeight;
  }

  // TODO can be better.
  auto matrix = glm::mat3{ 1.0 };
  matrix = glm::translate(matrix, offset);
  matrix = glm::rotate(matrix, rotate);
  matrix = glm::scale(matrix, { static_cast<float>(finalScaleX), static_cast<float>(finalScaleY) });
  return TransformHelper::toDesignMatrix(matrix);
}

TDesignMatrix TransformHelper::moveToWindowTopLeft(
  double               width,
  double               height,
  const TDesignMatrix& matrix)
{
  auto result = TransformHelper::getLTRB(width, height, matrix);
  return TransformHelper::translate(-result[0], -result[1], matrix);
}

TDesignMatrix TransformHelper::translate(double x, double y, const TDesignMatrix& matrix)
{
  return TransformHelper::toDesignMatrix(
    TransformHelper::fromDesignMatrix({ 1, 0, 0, 1, x, y }) *
    TransformHelper::fromDesignMatrix(matrix));
}

std::array<double, 4> TransformHelper::getLTRB(
  double               width,
  double               height,
  const TDesignMatrix& matrix)
{
  auto p0 = TransformHelper::calcXY(0, 0, matrix);
  auto p1 = TransformHelper::calcXY(width, 0, matrix);
  auto p2 = TransformHelper::calcXY(width, -height, matrix);
  auto p3 = TransformHelper::calcXY(0, -height, matrix);

  auto x = { std::get<0>(p0), std::get<0>(p1), std::get<0>(p2), std::get<0>(p3) };
  auto y = { std::get<1>(p0), std::get<1>(p1), std::get<1>(p2), std::get<1>(p3) };

  auto minX = std::min(x);
  auto maxX = std::max(x);

  auto minY = std::min(y);
  auto maxY = std::max(y);

  return { minX, maxY, maxX, minY };
}

std::array<double, 2> TransformHelper::getTranslate(const TRenderMatrix& renderMatrix)
{
  layer::Transform matrix(renderMatrix);
  return { matrix.translate()[0], matrix.translate()[1] };
}

std::array<double, 2> TransformHelper::getScale(const TRenderMatrix& renderMatrix)
{
  layer::Transform matrix(renderMatrix);
  return { matrix.scale()[0], matrix.scale()[1] };
}

double TransformHelper::getRotate(const TRenderMatrix& renderMatrix)
{
  layer::Transform matrix(renderMatrix);
  return matrix.rotate();
}

TRenderMatrix TransformHelper::createRenderMatrix(
  const std::array<double, 2>& translate,
  const std::array<double, 2>& scale,
  double                       rotate)
{
  glm::vec2 glmTranslate{ static_cast<float>(translate.at(0)),
                          static_cast<float>(translate.at(1)) };
  glm::vec2 glmScale{ static_cast<float>(scale.at(0)), static_cast<float>(scale.at(1)) };
  auto      glmRotate = static_cast<float>(rotate);

  auto matrix = glm::identity<glm::mat3>();
  matrix = glm::translate(matrix, glmTranslate);
  matrix = glm::rotate(matrix, glmRotate);
  matrix = glm::scale(matrix, glmScale);
  return matrix;
}

glm::mat3 TransformHelper::fromDesignMatrix(const TDesignMatrix& matrix)
{
  double a = matrix[0];
  double b = matrix[1];
  double c = matrix[2];
  double d = matrix[3];
  double tx = matrix[4];
  double ty = matrix[5];
  auto   renderMatrix = glm::mat3{ { a, b, 0 }, { c, d, 0 }, { tx, ty, 1 } };
  TransformHelper::changeYDirection(renderMatrix);
  return renderMatrix;
}

TDesignMatrix TransformHelper::toDesignMatrix(const TRenderMatrix& transform)
{
  auto m = transform;
  TransformHelper::changeYDirection(m);
  return { m[0][0], m[0][1], m[1][0], m[1][1], m[2][0], m[2][1] };
}

std::pair<double, double> TransformHelper::calcXY(double x, double y, const TDesignMatrix& matrix)
{
  double a = matrix[0];
  double b = matrix[1];
  double c = matrix[2];
  double d = matrix[3];
  double tx = matrix[4];
  double ty = matrix[5];

  double newX = a * x + c * y + tx;
  double newY = b * x + d * y + ty;
  return { newX, newY };
}
