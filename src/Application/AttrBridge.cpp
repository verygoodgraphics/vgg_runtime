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

layer::PaintNode* AttrBridge::nodeAt(const std::string& id)
{
  // TODO
  auto paintNode = m_layer->nodeAt(505, 105);
  return paintNode;
}

bool AttrBridge::updateColor(
  std::shared_ptr<LayoutNode>    node,
  size_t                         index,
  const Model::Color&            newColor,
  std::shared_ptr<NumberAnimate> animate)
{
  if (!node)
  {
    return false;
  }

  auto element = node->elementNode();
  if (!element)
  {
    return false;
  }

  auto paintNode = nodeAt(element->id());
  if (!paintNode)
  {
    return false;
  }

  auto object = element->object();
  if (!object)
  {
    return false;
  }

  std::vector<double> to{ newColor.alpha, newColor.red, newColor.green, newColor.blue };

  if (animate)
  {
    auto color = object->style.fills.at(index).color;
    animate->setFromTo({ color->alpha, color->red, color->green, color->blue }, to);
  }

  auto updateSource = [element, index](const std::vector<double>& value)
  {
    if (!element)
    {
      return;
    }

    auto object = element->object();
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

  auto updateDes = [paintNode, index](const std::vector<double>& value)
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

  if (!animate)
  {
    updateSource(to);
    updateDes(to);
  }
  else
  {
    animate->setAction(
      [updateSource, updateDes](const std::vector<double>& value)
      {
        updateSource(value);
        updateDes(value);
      });
    animate->start();
    m_animateManage.addAnimate(animate);
  }

  return true;
}
