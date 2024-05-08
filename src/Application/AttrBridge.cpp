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

std::optional<VGG::Color> AttrBridge::getFillColor(layer::PaintNode* node, size_t index)
{
  if (!node)
  {
    return {};
  }

  try
  {
    return std::get<VGG::Color>(node->attributeAccessor()->getFills().at(index).type);
  }
  catch (...)
  {
  }

  return {};
}

std::optional<double> AttrBridge::getOpacity(layer::PaintNode* node)
{
  if (!node)
  {
    return {};
  }

  return node->contextSetting().opacity;
}

std::optional<bool> AttrBridge::getVisible(layer::PaintNode* node)
{
  if (!node)
  {
    return {};
  }

  return node->isVisible();
}

std::optional<std::array<double, 6>> AttrBridge::getMatrix(layer::PaintNode* node)
{
  if (!node)
  {
    return {};
  }

  auto transform = node->attributeAccessor()->getTransform();
  return TransformHelper::toDesignMatrix(transform.matrix());
}

std::optional<double> AttrBridge::getWidth(layer::PaintNode* node)
{
  if (!node)
  {
    return {};
  }
  return node->frameBounds().width();
}

std::optional<double> AttrBridge::getHeight(layer::PaintNode* node)
{
  if (!node)
  {
    return {};
  }
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
  auto fills = node->attributeAccessor()->getFills();

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
  node->attributeAccessor()->setFills(fills);
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

void AttrBridge::setMatrix(
  std::shared_ptr<LayoutNode>  node,
  const std::array<double, 6>& designMatrix)
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

void AttrBridge::setMatrix(layer::PaintNode* node, const std::array<double, 6>& designMatrix)
{
  if (node)
  {
    node->attributeAccessor()->setTransform(
      VGG::layer::Transform(TransformHelper::fromDesignMatrix(designMatrix)));
  }
}

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

  const auto& id = element->id();

  // for top-level frame
  if (node->parent() && !node->parent()->parent())
  {
    for (auto& frame : m_view->getSceneNode()->getFrames())
    {
      if (frame->guid() == id)
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
  auto object = getlayoutNodeObject(node);
  if (!object)
  {
    return false;
  }

  if (!paintNode)
  {
    return false;
  }

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
    color = std::get<VGG::Color>(paintNode->attributeAccessor()->getFills().at(index).type);
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

  auto object = getlayoutNodeObject(node);
  if (!object)
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
  const std::array<double, 6>&   newMatrix,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate,
  bool                           isNotScaleButChangeSize)
{
  if (!paintNode)
  {
    return false;
  }

  auto oldMatrix = AttrBridge::getMatrix(paintNode);
  if (!oldMatrix)
  {
    assert(false);
    return false;
  }

  auto update =
    [node, paintNode, isOnlyUpdatePaint, isNotScaleButChangeSize](const std::vector<double>& value)
  {
    assert(value.size() == 5);

    glm::vec2 translate{ static_cast<float>(value.at(0)), static_cast<float>(value.at(1)) };
    glm::vec2 scale{ static_cast<float>(value.at(2)), static_cast<float>(value.at(3)) };
    auto      rotate = static_cast<float>(value.at(4));

    // TODO frame can not scale, should change width and height, what about group and symbol?
    if (isNotScaleButChangeSize)
    {
      scale = glm::vec2{ 1.0f, 1.0f };

      // TODO wait change size
    }

    auto matrix = glm::identity<glm::mat3>();
    matrix = glm::translate(matrix, translate);
    matrix = glm::rotate(matrix, rotate);
    matrix = glm::scale(matrix, scale);
    auto designMatrix = TransformHelper::toDesignMatrix(matrix);

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setMatrix(node, designMatrix);
    }

    AttrBridge::setMatrix(paintNode, designMatrix);
  };

  auto getInfo = [](layer::Transform transform)
  {
    return std::vector<double>{ transform.translate()[0],
                                transform.translate()[1],
                                transform.scale()[0],
                                transform.scale()[1],
                                transform.rotate() };
  };
  std::vector<double> from =
    getInfo(VGG::layer::Transform(TransformHelper::fromDesignMatrix(*oldMatrix)));
  std::vector<double> to =
    getInfo(VGG::layer::Transform(TransformHelper::fromDesignMatrix(newMatrix)));

  updateSimpleAttr(node, from, to, update, animate);

  return true;
}

void AttrBridge::setTwinMatrix(
  std::shared_ptr<LayoutNode> nodeFrom,
  std::shared_ptr<LayoutNode> nodeTo,
  layer::PaintNode*           paintNodeTo,
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

  auto boundsFrom = AttrBridge::getlayoutNodeObject(nodeFrom)->bounds;
  auto boundsTo = AttrBridge::getlayoutNodeObject(nodeTo)->bounds;

  double width = boundsFrom.width * scale[0];
  double height = boundsFrom.height * scale[1];

  if (boundsTo.width)
  {
    scale[0] = width / boundsTo.width;
  }
  if (boundsTo.height)
  {
    scale[1] = height / boundsTo.height;
  }

  if (ReplaceNodeAnimate::isContainerType(nodeFrom->elementNode()))
  {
    // TODO change nodeTo size
    scale = glm::vec2{ 1.0f, 1.0f };
  }

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

bool AttrBridge::replaceNode(
  const std::shared_ptr<LayoutNode>   oldNode,
  const std::shared_ptr<LayoutNode>   newNode,
  layer::PaintNode*                   oldPaintNode,
  layer::PaintNode*                   newPaintNode,
  bool                                isOnlyUpdatePaint,
  std::shared_ptr<ReplaceNodeAnimate> animate)
{
  auto removeOldPaintNodeIfNeed = [oldPaintNode, newPaintNode]()
  {
    if (!newPaintNode)
    {
      auto parent = oldPaintNode->parent();
      if (parent)
      {
        parent->removeChild(layer::incRef(oldPaintNode));
      }
    }
  };

  if (!newPaintNode)
  {
    if (!newNode || !oldPaintNode)
    {
      return false;
    }

    assert((!oldNode || !oldNode->parent()) && newNode->parent());

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

TransformHelper::TDesignMatrix TransformHelper::transform(
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

  auto finalScaleX = 0.0;
  if (selfWidth)
  {
    finalScaleX = desWidth * scale[0] / selfWidth;
  }

  auto finalScaleY = 0.0;
  if (selfHeight)
  {
    finalScaleY = desHeight * scale[1] / selfHeight;
  }

  auto matrix = glm::mat3{ 1.0 };
  matrix = glm::translate(matrix, offset);
  matrix = glm::rotate(matrix, rotate);
  matrix = glm::scale(matrix, { static_cast<float>(finalScaleX), static_cast<float>(finalScaleY) });
  return TransformHelper::toDesignMatrix(matrix);
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

TransformHelper::TDesignMatrix TransformHelper::toDesignMatrix(const TRenderMatrix& transform)
{
  auto m = transform;
  TransformHelper::changeYDirection(m);
  return { m[0][0], m[0][1], m[1][0], m[1][1], m[2][0], m[2][1] };
}
