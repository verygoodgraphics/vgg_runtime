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
#include "Domain/Layout/Node.hpp"
#include "Domain/Model/Element.hpp"
#include "Layer/VGGLayer.hpp"
#include "Utility/VggTimer.hpp"
#include "Application/Animate.hpp"
#include "Domain/Model/DesignModel.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/PaintNode.hpp"

using namespace VGG;

AttrBridge::AttrBridge(layer::VLayer* vLayer, AnimateManage& animateManage)
  : m_layer(vLayer)
  , m_animateManage(animateManage)
{
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

  // TODO
  // element->id();
  std::vector<VGG::layer::PaintNode*> result;
  m_layer->nodesAt(505, 105, result);
  if (node->elementNode()->id() == "1:6")
  {
    return result.at(0);
  }
  else
  {
    return result.at(1);
  }
}

std::optional<std::array<double, 6>> AttrBridge::getNodeMatrix(
  std::shared_ptr<LayoutNode> node,
  bool                        forPaintNode)
{
  auto result = std::array<double, 6>();

  if (forPaintNode)
  {
    auto paintNode = getPaintNode(node);
    if (!paintNode)
    {
      return {};
    }

    // TODO
    assert(false);
    return result;
  }
  else
  {
    auto object = AttrBridge::getlayoutNodeObject(node);
    if (!object)
    {
      return {};
    }

    if (object->matrix.size() != 6)
    {
      assert(false);
      return {};
    }

    std::copy(object->matrix.begin(), object->matrix.end(), result.begin());
    return result;
  }
}

std::optional<double> AttrBridge::getNodeOpacity(
  std::shared_ptr<LayoutNode> node,
  bool                        forPaintNode)
{
  if (forPaintNode)
  {
    auto paintNode = getPaintNode(node);
    if (!paintNode)
    {
      return {};
    }
    return paintNode->contextSetting().opacity;
  }
  else
  {
    auto object = AttrBridge::getlayoutNodeObject(node);
    if (!object)
    {
      return {};
    }
    return object->contextSettings.opacity;
  }
}

void AttrBridge::setNodeOpacity(std::shared_ptr<LayoutNode> node, double value, bool forPaintNode)
{
  if (forPaintNode)
  {
    auto paintNode = getPaintNode(node);
    if (!paintNode)
    {
      return;
    }

    auto contextSetting = paintNode->contextSetting();
    contextSetting.opacity = static_cast<float>(value);
    paintNode->setContextSettings(contextSetting);
  }
  else
  {
    auto object = AttrBridge::getlayoutNodeObject(node);
    if (!object)
    {
      return;
    }
    object->contextSettings.opacity = value;
  }

  return;
}

void AttrBridge::setNodeMatrix(
  std::shared_ptr<LayoutNode>  node,
  const std::array<double, 6>& value,
  bool                         forPaintNode)
{
  if (forPaintNode)
  {
    auto paintNode = getPaintNode(node);
    if (!paintNode)
    {
      return;
    }

    paintNode->attributeAccessor()->setTransform(TransformHelper::fromVggMatrix(value));
  }
  else
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

    std::copy(value.begin(), value.end(), object->matrix.begin());
  }

  return;
}

bool AttrBridge::updateSimpleAttr(
  std::shared_ptr<LayoutNode>                     node,
  layer::PaintNode*                               paintNode,
  const std::vector<double>&                      from,
  const std::vector<double>&                      to,
  std::function<void(const std::vector<double>&)> updateFrom,
  std::function<void(const std::vector<double>&)> updateTo,
  std::shared_ptr<NumberAnimate>                  animate)
{
  assert(paintNode);

  auto object = getlayoutNodeObject(node);
  if (!object)
  {
    return false;
  }

  if (!animate)
  {
    updateFrom(to);
    updateTo(to);
  }
  else
  {
    animate->setFromTo(from, to);
    animate->setAction(
      [updateFrom, updateTo](const std::vector<double>& value)
      {
        updateFrom(value);
        updateTo(value);
      });
    animate->start();
    m_animateManage.addAnimate(animate);
  }

  return true;
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

bool AttrBridge::updateColor(
  std::shared_ptr<LayoutNode>    node,
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

  auto paintNode = getPaintNode(node);
  if (!paintNode)
  {
    return false;
  }

  auto updateFrom = [node, index, isOnlyUpdatePaint](const std::vector<double>& value)
  {
    if (isOnlyUpdatePaint)
    {
      return;
    }

    auto object = AttrBridge::getlayoutNodeObject(node);
    if (!object || index >= object->style.fills.size())
    {
      return;
    }

    auto& color = object->style.fills.at(index).color;
    color->alpha = static_cast<float>(value.at(0));
    color->red = static_cast<float>(value.at(1));
    color->green = static_cast<float>(value.at(2));
    color->blue = static_cast<float>(value.at(3));
  };

  auto updateTo = [paintNode, index](const std::vector<double>& value)
  {
    // TODO 此处得用 get 的接口, 目前朔柳还没提供
    std::vector<Fill> attr(1);

    Color color;
    color.a = static_cast<float>(value.at(0));
    color.r = static_cast<float>(value.at(1));
    color.g = static_cast<float>(value.at(2));
    color.b = static_cast<float>(value.at(3));

    attr.front().type = color;
    paintNode->attributeAccessor()->setFills(attr);
  };

  auto color = object->style.fills.at(index).color;
  return updateSimpleAttr(
    node,
    paintNode,
    { color->alpha, color->red, color->green, color->blue },
    { newColor.alpha, newColor.red, newColor.green, newColor.blue },
    updateFrom,
    updateTo,
    animate);
}

bool AttrBridge::updateOpacity(
  std::shared_ptr<LayoutNode>    node,
  double                         newOpacity,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  auto paintNode = getPaintNode(node);
  if (!paintNode)
  {
    return false;
  }

  auto object = getlayoutNodeObject(node);
  if (!object)
  {
    return false;
  }

  auto update = [node, isOnlyUpdatePaint](
                  const std::vector<double>&  value,
                  bool                        forPaintNode,
                  std::shared_ptr<AttrBridge> self)
  {
    if (!forPaintNode && isOnlyUpdatePaint)
    {
      return;
    }

    assert(value.size() == 1);
    self->setNodeOpacity(node, value.at(0), forPaintNode);
  };

  updateSimpleAttr(
    node,
    paintNode,
    { object->contextSettings.opacity },
    { newOpacity },
    std::bind(update, std::placeholders::_1, false, shared_from_this()),
    std::bind(update, std::placeholders::_1, true, shared_from_this()),
    animate);

  return true;
}

bool AttrBridge::updateMatrix(
  std::shared_ptr<LayoutNode>    node,
  const std::array<double, 6>&   newMatrix,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  auto paintNode = getPaintNode(node);
  if (!paintNode)
  {
    return false;
  }

  auto oldMatrix = getNodeMatrix(node, false);
  if (!oldMatrix)
  {
    assert(false);
    return false;
  }

  auto update =
    [node, isOnlyUpdatePaint](const std::vector<double>& value, std::shared_ptr<AttrBridge> self)
  {
    assert(value.size() == 5);
    layer::Transform transform(
      glm::vec2{ value.at(0), value.at(1) },
      glm::vec2{ value.at(2), value.at(3) },
      static_cast<float>(value.at(4)));

    self->setNodeMatrix(node, TransformHelper::toVggMatrix(transform), true);
    if (!isOnlyUpdatePaint)
    {
      self->setNodeMatrix(node, TransformHelper::toVggMatrix(transform), false);
    }
  };

  auto getInfo = [](layer::Transform transform)
  {
    return std::vector<double>{ transform.translate()[0],
                                transform.translate()[1],
                                transform.scale()[0],
                                transform.scale()[1],
                                transform.rotate() };
  };
  std::vector<double> from = getInfo(TransformHelper::fromVggMatrix(*oldMatrix));
  std::vector<double> to = getInfo(TransformHelper::fromVggMatrix(newMatrix));

  updateSimpleAttr(
    node,
    paintNode,
    from,
    to,
    std::bind(update, std::placeholders::_1, shared_from_this()),
    std::bind(update, std::placeholders::_1, shared_from_this()),
    animate);

  return true;
}

bool AttrBridge::replaceNode(
  const std::shared_ptr<LayoutNode>   oldNode,
  const std::shared_ptr<LayoutNode>   newNode,
  bool                                correlateById,
  bool                                isOnlyUpdatePaint,
  std::shared_ptr<ReplaceNodeAnimate> animate)
{
  if (!animate)
  {
    // TODO
    setNodeOpacity(oldNode, 0, true);
    if (!isOnlyUpdatePaint)
    {
      setNodeOpacity(oldNode, 0, false);
    }
  }
  else
  {
    // TODO not change LayoutNode Attr
    animate->setFromTo(oldNode, newNode);
    animate->setIsOnlyUpdatePaint(isOnlyUpdatePaint);
    animate->start();
    m_animateManage.addAnimate(animate);
  }
  return true;
}

layer::Transform TransformHelper::fromVggMatrix(const TVggMatrix& matrix)
{
  VGG::layer::Transform transform({ glm::vec3{ matrix[0], matrix[1], 0 },
                                    glm::vec3{ matrix[2], matrix[3], 0 },
                                    glm::vec3{ matrix[4], matrix[5], 1 } });

  TransformHelper::changeYDirection(transform);
  return transform;
}

TransformHelper::TVggMatrix TransformHelper::toVggMatrix(layer::Transform transform)
{
  TransformHelper::changeYDirection(transform);
  auto matrix = transform.matrix();
  return { matrix[0][0], matrix[0][1], matrix[1][0], matrix[1][1], matrix[2][0], matrix[2][1] };
}

void TransformHelper::changeYDirection(layer::Transform& transform)
{
  glm::mat3 scale = glm::identity<glm::mat3>();
  scale = glm::scale(scale, { 1, -1 });
  transform = VGG::layer::Transform(scale * transform.matrix() * scale);
}