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
#include "Domain/Layout/LayoutNode.hpp"
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

  fills.at(index).contextSettings.opacity = static_cast<float>(value);
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
    bounds.setWidth(static_cast<float>(width));
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
    bounds.setHeight(static_cast<float>(height));
    node->setFrameBounds(bounds);
  }
}

void AttrBridge::updateSimpleAttr(
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
    animate->addTriggeredCallback([update](const std::vector<double>& value) { update(value); });
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

  auto sceneNode = m_view->getSceneNode();
  if (!sceneNode)
    return nullptr;

  const auto& id = element->idNumber();

  // for top-level frame
  if (node->parent() && !node->parent()->parent())
  {
    for (auto& frame : sceneNode->getFrames())
    {
      ASSERT(frame->node());
      if (frame->node()->uniqueID() == id)
      {
        return frame->node();
      }
    }
  }

  return sceneNode->nodeByID(id);
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

  updateSimpleAttr({ *nowValue }, { newOpacity }, update, animate);
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

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setOpacity(node, value.at(0));
    }

    AttrBridge::setOpacity(paintNode, value.at(0));
  };

  updateSimpleAttr({ *AttrBridge::getOpacity(paintNode) }, { newOpacity }, update, animate);

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

    auto realWidth = *originWidth;
    auto realHeight = *originHeight;

    if (isNotScaleButChangeSize)
    {
      realWidth = std::abs(scale[0] * (*originWidth));
      realHeight = std::abs(scale[1] * (*originHeight));

      AttrBridge::setWidth(paintNode, realWidth);
      AttrBridge::setHeight(paintNode, realHeight);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setWidth(node, realWidth);
        AttrBridge::setHeight(node, realHeight);
      }

      scale[0] = std::copysign(1, scale[0]);
      scale[1] = std::copysign(1, scale[1]);
    }

    auto matrix = TransformHelper::createRenderMatrix(
      realWidth,
      realHeight,
      value[0],
      value[1],
      scale[0],
      scale[1],
      value[4]);

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

        // invMatrixs *=
        // layer::Transform(TransformHelper::fromDesignMatrix(*parentMatrix)).inverse();
        invMatrixs *= glm::inverse(TransformHelper::fromDesignMatrix(*parentMatrix));
        parent = parent->parent();
      }

      // finalMatrix = TransformHelper::toDesignMatrix(layer::Transform(invMatrixs *
      // matrix).matrix());
      finalMatrix = TransformHelper::toDesignMatrix(invMatrixs * matrix);
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

  auto getInfo = [](const TRenderMatrix& transform)
  {
    auto value = TransformHelper::getTSR(transform);
    return std::vector<double>{ value[2], value[3], value[4] };
  };

  std::vector<double> oldMatrixInfo = getInfo(TransformHelper::fromDesignMatrix(*oldMatrix));
  std::vector<double> newMatrixInfo = getInfo(TransformHelper::fromDesignMatrix(newMatrix));

  auto centerOld = TransformHelper::calcXY(*originWidth / 2.0, -*originHeight / 2.0, *oldMatrix);
  auto centerNew = TransformHelper::calcXY(*originWidth / 2.0, -*originHeight / 2.0, newMatrix);

  std::vector<double> from{ std::get<0>(centerOld),
                            -std::get<1>(centerOld),
                            oldMatrixInfo[0],
                            oldMatrixInfo[1],
                            oldMatrixInfo[2] };

  std::vector<double> to{ std::get<0>(centerNew),
                          -std::get<1>(centerNew),
                          newMatrixInfo[0],
                          newMatrixInfo[1],
                          newMatrixInfo[2] };

  updateSimpleAttr(from, to, update, animate);

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
  std::shared_ptr<LayoutNode> nodeTo,
  layer::PaintNode*           paintNodeTo,
  double                      originalWidthFrom,
  double                      originalHeightFrom,
  double                      originWidthTo,
  double                      originHeightTo,
  const std::vector<double>&  value,
  bool                        isOnlyUpdatePaint,
  bool                        isNotScaleButChangeSize)
{
  if (!nodeTo || !paintNodeTo)
  {
    assert(false);
    return;
  }

  std::array<double, 2> scale{ value[2], value[3] };
  double                width = originalWidthFrom * scale[0];
  double                height = originalHeightFrom * scale[1];

  if (originWidthTo)
  {
    scale[0] = width / originWidthTo;
  }
  if (originHeightTo)
  {
    scale[1] = height / originHeightTo;
  }

  auto realWidth = originWidthTo;
  auto realHeight = originHeightTo;

  if (isNotScaleButChangeSize)
  {
    realWidth = std::abs(scale[0] * originWidthTo);
    realHeight = std::abs(scale[1] * originHeightTo);

    AttrBridge::setWidth(paintNodeTo, realWidth);
    AttrBridge::setHeight(paintNodeTo, realHeight);

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setWidth(nodeTo, realWidth);
      AttrBridge::setHeight(nodeTo, realHeight);
    }

    scale[0] = std::copysign(1, scale[0]);
    scale[1] = std::copysign(1, scale[1]);
  }

  auto matrix = TransformHelper::createRenderMatrix(
    realWidth,
    realHeight,
    value[0],
    value[1],
    scale[0],
    scale[1],
    value[4]);

  auto designMatrix = TransformHelper::toDesignMatrix(matrix);

  if (!isOnlyUpdatePaint)
  {
    AttrBridge::setMatrix(nodeTo, designMatrix);
  }

  AttrBridge::setMatrix(paintNodeTo, designMatrix);
}

void AttrBridge::setContentOffset(std::shared_ptr<LayoutNode> node, const double x, const double y)
{
  // TODO
}

void AttrBridge::setContentOffset(layer::PaintNode* node, const double x, const double y)
{
  // TODO
  // auto matrix =
  //   TransformHelper::fromDesignMatrix(TransformHelper::translate(x, y, { 1, 0, 0, 1, 0, 0 }));
  // node->setContentTransform(layer::Transform(matrix));
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

  updateSimpleAttr({ oldWidth, oldHeight }, { newWidth, newHeight }, update, animate);
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

  // TODO use AttrBridge::delChild will be better.
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

  // TODO use AttrBridge::addChild will be better.
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

bool AttrBridge::addChild(
  const std::shared_ptr<LayoutNode> container,
  const std::shared_ptr<LayoutNode> newNode,
  layer::PaintNode*                 containerPaintNode,
  size_t                            index,
  std::shared_ptr<NumberAnimate>    animate)
{
  if (!container || !newNode || !containerPaintNode)
  {
    return false;
  }

  auto& children = containerPaintNode->children();

  auto size = children.size();
  if (index == static_cast<size_t>(-1))
  {
    index = size;
  }

  if (children.size() < index)
  {
    return false;
  }

  auto element = newNode->elementNode();
  if (!element)
  {
    return false;
  }

  auto                      type = element->type();
  layer::SceneBuilderResult result;

  switch (type)
  {
    case VGG::Domain::Element::EType::PATH:
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelPath>(
        { layer::StructPathObject(element) });
      break;
    }
    case VGG::Domain::Element::EType::IMAGE:
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelImage>(
        { layer::StructImageObject(element) });
      break;
    }
    case VGG::Domain::Element::EType::TEXT:
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelText>(
        { layer::StructTextObject(element) });
      break;
    }
    case VGG::Domain::Element::EType::GROUP:
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelGroup>(
        { layer::StructGroupObject(element) });
      break;
    }
    case VGG::Domain::Element::EType::FRAME:
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelFrame>(
        { layer::StructFrameObject(element) });
      break;
    }
    case VGG::Domain::Element::EType::SYMBOL_MASTER:
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelMaster>(
        { layer::StructMasterObject(element) });
      break;
    }
    case VGG::Domain::Element::EType::SYMBOL_INSTANCE:
    {
      result = layer::SceneBuilder::builder().build<layer::StructModelInstance>(
        { layer::StructInstanceObject(element) });
      break;
    }
    default:
    {
      assert(false);
      return false;
    }
  }

  if (!result.root || result.root->size() != 1)
  {
    return false;
  }

  auto newPaintNode = result.root->at(0)->node();
  if (!newPaintNode)
  {
    return false;
  }

  containerPaintNode->addChild(children.begin() + index, layer::incRef(newPaintNode));

  if (animate)
  {
    auto opacity = AttrBridge::getOpacity(newPaintNode);
    assert(opacity);

    updateOpacity(newNode, newPaintNode, 0, true);
    updateOpacity(newNode, newPaintNode, *opacity, true, animate);
  }

  return true;
}

bool AttrBridge::delChild(
  layer::PaintNode*              paintNode,
  size_t                         index,
  std::shared_ptr<NumberAnimate> animate)
{
  auto& children = paintNode->children();
  if (index >= children.size())
  {
    return false;
  }

  if (!animate)
  {
    paintNode->removeChild(children[index]);
  }
  else
  {
    updateOpacity(nullptr, children[index].get(), 0, true, animate);
    animate->addCallBackWhenStop(
      [paintNode, index]()
      {
        // Note: for safe, should not call this->delChild direct.
        auto& children = paintNode->children();
        if (index >= children.size())
        {
          return;
        }

        paintNode->removeChild(children[index]);
      });
  }

  return true;
}

bool AttrBridge::scrollTo(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  std::array<double, 2>          oldValue,
  std::array<double, 2>          newValue,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  if (!paintNode)
  {
    return false;
  }

  auto update = [node, paintNode, isOnlyUpdatePaint](const std::vector<double>& value)
  {
    assert(value.size() == 2);

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setContentOffset(node, value.at(0), value.at(1));
    }

    AttrBridge::setContentOffset(paintNode, value.at(0), value.at(1));
  };

  updateSimpleAttr({ oldValue[0], oldValue[1] }, { newValue[0], newValue[1] }, update, animate);
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
  auto renderMatrix = TransformHelper::fromDesignMatrix(desMatrix);
  auto value = TransformHelper::getTSR(renderMatrix);

  auto finalScaleX = 0.0;
  if (selfWidth)
  {
    finalScaleX = desWidth * value[2] / selfWidth;
  }

  auto finalScaleY = 0.0;
  if (selfHeight)
  {
    finalScaleY = desHeight * value[3] / selfHeight;
  }

  return TransformHelper::toDesignMatrix(TransformHelper::createRenderMatrix(
    { value[0], value[1] },
    { finalScaleX, finalScaleY },
    value[4]));
}

TDesignMatrix TransformHelper::moveToWindowTopLeft(
  double               boundOriginX,
  double               boundOriginY,
  double               width,
  double               height,
  const TDesignMatrix& matrix)
{
  auto result = TransformHelper::getLTRB(width, height, matrix);
  return TransformHelper::translate(
    -(result[0] + boundOriginX),
    -(result[1] + boundOriginY),
    matrix);
  // return TransformHelper::translate(-result[0], -result[1], matrix);
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

// std::array<double, 2> TransformHelper::getTranslate(const TRenderMatrix& renderMatrix)
//{
//   layer::Transform matrix(renderMatrix);
//   return { matrix.translate()[0], matrix.translate()[1] };
// }
//
// std::array<double, 2> TransformHelper::getScale(const TRenderMatrix& renderMatrix)
//{
//   layer::Transform matrix(renderMatrix);
//   return { matrix.scale()[0], matrix.scale()[1] };
// }
//
// double TransformHelper::getRotate(const TRenderMatrix& renderMatrix)
//{
//   layer::Transform matrix(renderMatrix);
//   return matrix.rotate();
// }

std::array<double, 5> TransformHelper::getTSR(const TRenderMatrix& renderMatrix)
{
  double a = renderMatrix[0][0];
  double b = renderMatrix[0][1];
  double c = renderMatrix[1][0];
  double d = renderMatrix[1][1];

  double lengthOfScaleX = std::hypot(a, b);
  double scaleX[2] = { lengthOfScaleX, -lengthOfScaleX };

  double lengthOfScaleY = std::hypot(c, d);
  double scaleY[2] = { lengthOfScaleY, -lengthOfScaleY };

  if (!lengthOfScaleX || !lengthOfScaleY)
  {
    assert(false);
    return { renderMatrix[2][0], renderMatrix[2][1], 1.0, 1.0, 0.0 };
  }

  double r0[2] = { std::atan2(b / scaleX[0], a / scaleX[0]),
                   std::atan2(b / scaleX[1], a / scaleX[1]) };

  double r1[2] = { std::atan2(-c / scaleY[0], d / scaleY[0]),
                   std::atan2(-c / scaleY[1], d / scaleY[1]) };

  double realScaleX = 0;
  double realScaleY = 0;
  double realRotation = 0;
  if (std::abs(r0[0] - r1[0]) < 0.001)
  {
    realScaleX = scaleX[0];
    realScaleY = scaleY[0];
    realRotation = r0[0];

    // assert(std::abs(r0[0] - r1[1]) > 0.001);
    // assert(std::abs(r0[1] - r1[0]) > 0.001);
    // assert(std::abs(r0[1] - r1[1]) > 0.001);
  }
  else if (std::abs(r0[0] - r1[1]) < 0.001)
  {
    realScaleX = scaleX[0];
    realScaleY = scaleY[1];
    realRotation = r0[0];

    // assert(std::abs(r0[0] - r1[0]) > 0.001);
    // assert(std::abs(r0[1] - r1[0]) > 0.001);
    // assert(std::abs(r0[1] - r1[1]) > 0.001);
  }
  else if (std::abs(r0[1] - r1[0]) < 0.001)
  {
    realScaleX = scaleX[1];
    realScaleY = scaleY[0];
    realRotation = r0[1];

    // assert(std::abs(r0[0] - r1[0]) > 0.001);
    // assert(std::abs(r0[0] - r1[1]) > 0.001);
    // assert(std::abs(r0[1] - r1[1]) > 0.001);
  }
  else
  {
    assert(std::abs(r0[1] - r1[1]) < 0.001);
    realScaleX = scaleX[1];
    realScaleY = scaleY[1];
    realRotation = r0[1];
  }

  return { renderMatrix[2][0], renderMatrix[2][1], realScaleX, realScaleY, realRotation };
}

TRenderMatrix TransformHelper::createRenderMatrix(
  const std::array<double, 2>& translate,
  const std::array<double, 2>& scale,
  double                       rotate)
{
  auto dealScale = [](double value)
  {
    if (std::abs(value) < 0.000001)
    {
      if (value > 0)
      {
        return 0.000001f;
      }

      return -0.000001f;
    }

    return static_cast<float>(value);
  };

  auto scaleX = dealScale(scale.at(0));
  auto scaleY = dealScale(scale.at(1));

  glm::vec2 glmTranslate{ static_cast<float>(translate.at(0)),
                          static_cast<float>(translate.at(1)) };
  glm::vec2 glmScale{ scaleX, scaleY };
  auto      glmRotate = static_cast<float>(rotate);

  auto matrix = glm::identity<glm::mat3>();
  matrix = glm::translate(matrix, glmTranslate);
  matrix = glm::rotate(matrix, glmRotate);
  matrix = glm::scale(matrix, glmScale);
  return matrix;
}

TRenderMatrix TransformHelper::createRenderMatrix(
  double width,
  double height,
  double centerXInFatherCoordinateSystem,
  double centerYInFatherCoordinateSystem,
  double scaleX,
  double scaleY,
  double rotate)
{
  auto m = glm::identity<glm::mat3>();
  m = glm::rotate(m, static_cast<float>(rotate));
  m = glm::scale(m, { static_cast<float>(scaleX), static_cast<float>(scaleY) });

  glm::vec3 center{ static_cast<float>(width / 2.0), static_cast<float>(height / 2.0), 1.0f };

  center = m * center;
  auto tx = centerXInFatherCoordinateSystem - center[0];
  auto ty = centerYInFatherCoordinateSystem - center[1];

  return TransformHelper::createRenderMatrix({ tx, ty }, { scaleX, scaleY }, rotate);
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
