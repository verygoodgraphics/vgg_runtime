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
#include "Layer/Core/VType.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Memory/Ref.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/SceneBuilder.hpp"
#include "Layer/Model/StructModelSerde.hpp"
#include "Layer/CoordinateConvert.hpp"
#include <map>

using namespace VGG;

const std::map<int, VGG::EBlendMode> g_blendMode{
  { 0, VGG::BM_NORMAL },        { 1, VGG::BM_DARKEN },       { 2, VGG::BM_MULTIPLY },
  { 3, VGG::BM_COLOR_BURN },    { 4, VGG::BM_LIGHTEN },      { 5, VGG::BM_SCREEN },
  { 6, VGG::BM_COLOR_DODGE },   { 7, VGG::BM_OVERLAY },      { 8, VGG::BM_SOFT_LIGHT },
  { 9, VGG::BM_HARD_LIGHT },    { 10, VGG::BM_DIFFERENCE },  { 11, VGG::BM_EXCLUSION },
  { 12, VGG::BM_HUE },          { 13, VGG::BM_SATURATION },  { 14, VGG::BM_COLOR },
  { 15, VGG::BM_LUMINOSITY },   { 16, VGG::BM_PLUS_DARKER }, { 17, VGG::BM_PLUS_LIGHTER },
  { 27, VGG::BM_PASS_THROUGHT }
};

#define CHECK_EXPR(expr, resultWhenFailed)                                                         \
  if (!(expr))                                                                                     \
  {                                                                                                \
    return resultWhenFailed;                                                                       \
  }

#define GET_PAINTNODE_ACCESSOR(node, accessor, resultWhenFailed)                                   \
  CHECK_EXPR(node, resultWhenFailed)                                                               \
  auto accessor = node->attributeAccessor();                                                       \
  CHECK_EXPR(accessor, resultWhenFailed)

template<typename T, typename U>
bool getPatternOrGradientAttr(layer::PaintNode* node, size_t index, bool effectOnFill, T fun)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, false);

  if (effectOnFill)
  {
    if (index >= *AttrBridge::getFillSize(node))
    {
      return false;
    }

    auto& object = std::get<U>(accessor->getFills().at(index).type);
    std::visit(fun, object.instance);
  }
  else
  {
    // TODO not complete.
    return false;
  }

  return true;
}

template<typename T, typename U>
void setPatternOrGradientAttr(layer::PaintNode* node, size_t index, bool effectOnFill, T fun)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, void());

  if (effectOnFill)
  {
    auto fills = accessor->getFills();
    if (index >= fills.size())
    {
      return;
    }

    try
    {
      auto& object = std::get<U>(fills.at(index).type);
      std::visit(fun, object.instance);
    }
    catch (...)
    {
      return;
    }

    accessor->setFills(fills);
  }
  else
  {
    // TODO not complete
  }
}

template<typename T>
bool getPatternAttr(layer::PaintNode* node, size_t index, bool effectOnFill, T fun)
{
  return getPatternOrGradientAttr<T, VGG::Pattern>(node, index, effectOnFill, fun);
}

template<typename T>
void setLayoutNodePatternAttr(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  bool                        effectOnFill,
  T                           fun)
{
  if (auto object = AttrBridge::getlayoutNodeObject(node))
  {
    if (effectOnFill)
    {
      if (index >= object->style.fills.size())
      {
        return;
      }

      auto& pattern = object->style.fills.at(index).pattern;
      if (!pattern)
      {
        return;
      }

      std::visit(fun, pattern->instance);
    }
    else
    {
      // TODO not complete
    }
  }
}

template<typename T>
void setPaintNodePatternAttr(layer::PaintNode* node, size_t index, bool effectOnFill, T fun)
{
  setPatternOrGradientAttr<T, VGG::Pattern>(node, index, effectOnFill, fun);
}

template<typename T>
bool getGradientAttr(layer::PaintNode* node, size_t index, bool effectOnFill, T fun)
{
  return getPatternOrGradientAttr<T, VGG::Gradient>(node, index, effectOnFill, fun);
}

template<typename T>
void setLayoutNodeGradientAttr(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  bool                        effectOnFill,
  T                           fun)
{
  if (auto object = AttrBridge::getlayoutNodeObject(node))
  {
    if (effectOnFill)
    {
      if (index >= object->style.fills.size())
      {
        return;
      }

      auto& gradient = object->style.fills.at(index).gradient;
      if (!gradient)
      {
        return;
      }

      fun(gradient->instance);
    }
    else
    {
      // TODO not complete
    }
  }
}

template<typename T>
void setPaintNodeGradientAttr(layer::PaintNode* node, size_t index, bool effectOnFill, T fun)
{
  setPatternOrGradientAttr<T, VGG::Gradient>(node, index, effectOnFill, fun);
}

AttrBridge::AttrBridge(std::shared_ptr<UIView> view, AnimateManage& animateManage)
  : m_view(view)
  , m_animateManage(animateManage)
{
}

std::optional<glm::vec4> AttrBridge::getFillColor(layer::PaintNode* node, size_t index)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});

  try
  {
    return std::get<glm::vec4>(accessor->getFills().at(index).type);
  }
  catch (...)
  {
  }

  return {};
}

std::optional<bool> AttrBridge::getFillEnabled(layer::PaintNode* node, size_t index)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});

  try
  {
    return accessor->getFills().at(index).isEnabled;
  }
  catch (...)
  {
  }

  return {};
}

std::optional<int> AttrBridge::getFillBlendMode(layer::PaintNode* node, size_t index)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});

  try
  {
    // g_blendMode does not have a large number of elements, so linear search is acceptable.
    auto value = accessor->getFills().at(index).contextSettings.blendMode;
    auto it = std::find_if(
      g_blendMode.begin(),
      g_blendMode.end(),
      [value](const std::pair<int, VGG::EBlendMode>& item) { return item.second == value; });

    if (it != g_blendMode.end())
    {
      return it->first;
    }
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

std::optional<int> AttrBridge::getFillType(layer::PaintNode* node, size_t index)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, {});

  try
  {
    auto id = accessor->getFills().at(index).type.index();
    if (!id)
    {
      return 1;
    }
    else if (id == 1)
    {
      return 2;
    }
    else
    {
      assert(id == 2);
      return 0;
    }
  }
  catch (...)
  {
  }

  return {};
}

std::optional<double> AttrBridge::getOpacity(layer::PaintNode* node)
{
  CHECK_EXPR(node, {});
  return node->contextSetting().opacity;
}

std::optional<std::string> AttrBridge::getFillPatternType(layer::PaintNode* node, size_t index)
{
  static const std::string names[4] = {
    "patternImageFill",
    "patternImageFit",
    "patternImageStretch",
    "patternImageTile",
    // "patternLayer" // render not complete.
  };

  GET_PAINTNODE_ACCESSOR(node, accessor, {});

  try
  {
    auto id = std::get<VGG::Pattern>(accessor->getFills().at(index).type).instance.index();
    if (id < 4)
    {
      return names[id];
    }
  }
  catch (...)
  {
  }

  return {};
}

std::optional<std::string> AttrBridge::getPatternImageFileName(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  std::string name;

  if (!getPatternAttr(node, index, effectOnFill, [&name](auto&& arg) { name = arg.guid; }))
  {
    return {};
  }

  return name;
}

std::optional<VGG::ImageFilter> AttrBridge::getPatternImageFilters(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  VGG::ImageFilter filter;

  if (!getPatternAttr(
        node,
        index,
        effectOnFill,
        [&filter](auto&& arg) { filter = arg.imageFilter; }))
  {
    return {};
  }

  return filter;
}

std::optional<double> AttrBridge::getPatternRotation(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  double rotation = 0;

  auto fun = [&rotation](auto&& arg)
  {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, VGG::PatternStretch>)
    {
      assert(false);
    }
    else
    {
      rotation = -arg.rotation;
    }
  };

  if (!getPatternAttr(node, index, effectOnFill, fun))
  {
    return {};
  }

  return rotation;
}

std::optional<bool> AttrBridge::getPatternImageTileMirror(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  bool mirror = false;

  auto fun = [&mirror](auto&& arg)
  {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (!std::is_same_v<T, VGG::PatternTile>)
    {
      assert(false);
    }
    else
    {
      mirror = arg.mirror;
    }
  };

  if (!getPatternAttr(node, index, effectOnFill, fun))
  {
    return {};
  }

  return mirror;
}

std::optional<double> AttrBridge::getPatternImageTileScale(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  double scale = 1.0;

  auto fun = [&scale](auto&& arg)
  {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (!std::is_same_v<T, VGG::PatternTile>)
    {
      assert(false);
    }
    else
    {
      scale = arg.scale;
    }
  };

  if (!getPatternAttr(node, index, effectOnFill, fun))
  {
    return {};
  }

  return scale;
}

std::optional<int> AttrBridge::getPatternImageTileMode(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  int mode = 0;

  auto fun = [&mode](auto&& arg)
  {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (!std::is_same_v<T, VGG::PatternTile>)
    {
      assert(false);
    }
    else
    {
      if (arg.mode == VGG::ETilePatternType::TILE_BOTH)
      {
        mode = 0;
      }
      else if (arg.mode == VGG::ETilePatternType::TILE_HORIZONTAL)
      {
        mode = 1;
      }
      else
      {
        assert(arg.mode == VGG::ETilePatternType::TILE_VERTICAL);
        mode = 2;
      }
    }
  };

  if (!getPatternAttr(node, index, effectOnFill, fun))
  {
    return {};
  }

  return mode;
}

std::optional<std::array<double, 2>> AttrBridge::getGradientFromOrTo(
  layer::PaintNode* node,
  size_t            index,
  bool              forFrom,
  bool              effectOnFill)
{
  double x = 0;
  double y = 0;

  auto fun = [&x, &y, forFrom](auto&& arg)
  {
    if (forFrom)
    {
      x = arg.from[0];
      y = -arg.from[1];
    }
    else
    {
      x = arg.to[0];
      y = -arg.to[1];
    }
  };

  if (!getGradientAttr(node, index, effectOnFill, fun))
  {
    return {};
  }

  return std::array<double, 2>{ x, y };
}

std::optional<std::variant<double, std::array<double, 2>>> AttrBridge::getGradientEllipse(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  std::optional<std::variant<double, std::array<double, 2>>> result;

  auto fun = [&result](auto&& arg)
  {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, VGG::GradientLinear>)
    {
      assert(false);
    }
    else
    {
      const auto& item = arg.ellipse;
      if (!item.index())
      {
        result = std::get<float>(item);
      }
      else
      {
        const auto& point = std::get<glm::vec2>(item);
        result = std::array<double, 2>{ point[0], -point[1] };
      }
    }
  };

  if (!getGradientAttr(node, index, effectOnFill, fun))
  {
    return {};
  }

  return result;
}

std::optional<size_t> AttrBridge::getGradientStopsCount(
  layer::PaintNode* node,
  size_t            index,
  bool              effectOnFill)
{
  size_t size = 0;

  if (!getGradientAttr(node, index, effectOnFill, [&size](auto&& arg) { size = arg.stops.size(); }))
  {
    return {};
  }

  return size;
}

std::optional<double> AttrBridge::getGradientStopsPosition(
  layer::PaintNode* node,
  size_t            index,
  size_t            indexForStops,
  bool              effectOnFill)
{
  auto size = getGradientStopsCount(node, index, effectOnFill);
  if (!size)
  {
    return {};
  }

  if (indexForStops >= *size)
  {
    return {};
  }

  double position = 0;

  if (!getGradientAttr(
        node,
        index,
        effectOnFill,
        [&position, indexForStops](auto&& arg)
        { position = arg.stops.at(indexForStops).position; }))
  {
    return {};
  }

  return position;
}

std::optional<glm::vec4> AttrBridge::getGradientStopsColor(
  layer::PaintNode* node,
  size_t            index,
  size_t            indexForStops,
  bool              effectOnFill)
{
  auto size = getGradientStopsCount(node, index, effectOnFill);
  if (!size)
  {
    return {};
  }

  if (indexForStops >= *size)
  {
    return {};
  }

  glm::vec4 color;

  if (!getGradientAttr(
        node,
        index,
        effectOnFill,
        [&color, indexForStops](auto&& arg) { color = arg.stops.at(indexForStops).color; }))
  {
    return {};
  }

  return color;
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

std::optional<size_t> AttrBridge::getChildrenSize(layer::PaintNode* node)
{
  CHECK_EXPR(node, {});
  return node->children().size();
}

std::optional<std::array<double, 2>> AttrBridge::getScrollInfo(layer::PaintNode* node)
{
  auto tsr = TransformHelper::getTSR(node->getContentTransform().matrix());
  return std::array<double, 2>{ tsr[0], -tsr[1] };
}

void AttrBridge::setFillEnabled(std::shared_ptr<LayoutNode> node, size_t index, bool enabled)
{
  auto object = AttrBridge::getlayoutNodeObject(node);
  if (!object || index >= object->style.fills.size())
  {
    return;
  }

  object->style.fills.at(index).isEnabled = enabled;
}

void AttrBridge::setFillEnabled(layer::PaintNode* node, size_t index, bool enabled)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, void());

  auto fills = accessor->getFills();
  if (index >= fills.size())
  {
    return;
  }

  fills.at(index).isEnabled = enabled;
  accessor->setFills(fills);
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

  glm::vec4 color;
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

void AttrBridge::setFillBlendMode(std::shared_ptr<LayoutNode> node, size_t index, int value)
{
  if (auto object = AttrBridge::getlayoutNodeObject(node))
  {
    if (index >= object->style.fills.size())
    {
      return;
    }
    object->style.fills.at(index).contextSettings.blendMode = value;
  }
}

void AttrBridge::setFillBlendMode(layer::PaintNode* node, size_t index, const int value)
{
  GET_PAINTNODE_ACCESSOR(node, accessor, void());

  auto fills = accessor->getFills();
  if (index >= fills.size())
  {
    return;
  }

  fills.at(index).contextSettings.blendMode = static_cast<VGG::EBlendMode>(value);
  accessor->setFills(fills);
}

void AttrBridge::setPatternImageFileName(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  const std::string&          newName,
  bool                        effectOnFill)
{
  setLayoutNodePatternAttr(
    node,
    index,
    effectOnFill,
    [&newName](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (
        std::is_same_v<T, std::monostate> || std::is_same_v<T, VGG::Model::PatternLayerInstance>)
      {
        assert(false);
      }
      else
      {
        arg.imageFileName = newName;
      }
    });
}

void AttrBridge::setPatternImageFileName(
  layer::PaintNode*  node,
  size_t             index,
  const std::string& newName,
  bool               effectOnFill)
{
  setPaintNodePatternAttr(
    node,
    index,
    effectOnFill,
    [&newName](auto&& arg) { arg.guid = newName; });
}

void AttrBridge::setPatternImageFilters(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  const std::vector<double>&  value,
  bool                        effectOnFill)
{
  setLayoutNodePatternAttr(
    node,
    index,
    effectOnFill,
    [&value](auto&& arg)
    {
      assert(value.size() == 8);

      using T = std::decay_t<decltype(arg)>;
      if constexpr (
        std::is_same_v<T, std::monostate> || std::is_same_v<T, VGG::Model::PatternLayerInstance>)
      {
        assert(false);
      }
      else
      {
        int   i = 0;
        auto& filter = arg.imageFilters;
        if (!filter)
        {
          filter = VGG::Model::ImageFilters();
        }
        filter->exposure = value.at(i++);
        filter->contrast = value.at(i++);
        filter->saturation = value.at(i++);
        filter->temperature = value.at(i++);
        filter->tint = value.at(i++);
        filter->highlights = value.at(i++);
        filter->shadows = value.at(i++);
        filter->hue = value.at(i++);
        filter->isEnabled = true;
      }
    });
}

void AttrBridge::setPatternImageFilters(
  layer::PaintNode*          node,
  size_t                     index,
  const std::vector<double>& value,
  bool                       effectOnFill)
{
  setPaintNodePatternAttr(
    node,
    index,
    effectOnFill,
    [&value](auto&& arg)
    {
      assert(value.size() == 8);

      int   i = 0;
      auto& filter = arg.imageFilter;
      filter.exposure = static_cast<float>(value.at(i++));
      filter.contrast = static_cast<float>(value.at(i++));
      filter.saturation = static_cast<float>(value.at(i++));
      filter.temperature = static_cast<float>(value.at(i++));
      filter.tint = static_cast<float>(value.at(i++));
      filter.highlight = static_cast<float>(value.at(i++));
      filter.shadow = static_cast<float>(value.at(i++));
      filter.hue = static_cast<float>(value.at(i++));
    });
}

void AttrBridge::setPatternRotation(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  double                      value,
  bool                        effectOnFill)
{
  setLayoutNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (
        std::is_same_v<T, std::monostate> || std::is_same_v<T, VGG::Model::PatternLayerInstance> ||
        std::is_same_v<T, VGG::Model::PatternImageStrech>)
      {
        assert(false);
      }
      else
      {
        arg.rotation = value;
      }
    });
}

void AttrBridge::setPatternRotation(
  layer::PaintNode* node,
  size_t            index,
  double            value,
  bool              effectOnFill)
{
  setPaintNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, VGG::PatternStretch>)
      {
        assert(false);
      }
      else
      {
        arg.rotation = -value;
      }
    });
}

void AttrBridge::setPatternImageTileMirror(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  bool                        value,
  bool                        effectOnFill)
{
  setLayoutNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!std::is_same_v<T, VGG::Model::PatternImageTile>)
      {
        assert(false);
      }
      else
      {
        arg.mirror = value;
      }
    });
}

void AttrBridge::setPatternImageTileMirror(
  layer::PaintNode* node,
  size_t            index,
  bool              value,
  bool              effectOnFill)
{
  setPaintNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!std::is_same_v<T, VGG::PatternTile>)
      {
        assert(false);
      }
      else
      {
        arg.mirror = value;
      }
    });
}

void AttrBridge::setPatternImageTileScale(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  double                      value,
  bool                        effectOnFill)
{
  setLayoutNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!std::is_same_v<T, VGG::Model::PatternImageTile>)
      {
        assert(false);
      }
      else
      {
        arg.scale = value;
      }
    });
}

void AttrBridge::setPatternImageTileScale(
  layer::PaintNode* node,
  size_t            index,
  double            value,
  bool              effectOnFill)
{
  setPaintNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!std::is_same_v<T, VGG::PatternTile>)
      {
        assert(false);
      }
      else
      {
        arg.scale = value;
      }
    });
}

void AttrBridge::setPatternImageTileMode(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  int                         value,
  bool                        effectOnFill)
{
  setLayoutNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!std::is_same_v<T, VGG::Model::PatternImageTile>)
      {
        assert(false);
      }
      else
      {
        arg.mode = value;
      }
    });
}

void AttrBridge::setPatternImageTileMode(
  layer::PaintNode* node,
  size_t            index,
  int               value,
  bool              effectOnFill)
{
  setPaintNodePatternAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!std::is_same_v<T, VGG::PatternTile>)
      {
        assert(false);
      }
      else
      {
        if (!value)
        {
          arg.mode = VGG::ETilePatternType::TILE_BOTH;
        }
        else if (1 == value)
        {
          arg.mode = VGG::ETilePatternType::TILE_HORIZONTAL;
        }
        else
        {
          assert(value == 2);
          arg.mode = VGG::ETilePatternType::TILE_VERTICAL;
        }
      }
    });
}

void AttrBridge::setGradientFromOrTo(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  double                      newX,
  double                      newY,
  bool                        forFrom,
  bool                        effectOnFill)
{
  setLayoutNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [newX, newY, forFrom](VGG::Model::GradientInstance& ins)
    {
      if (forFrom)
      {
        ins.from = { newX, newY };
      }
      else
      {
        ins.to = { newX, newY };
      }
    });
}

void AttrBridge::setGradientFromOrTo(
  layer::PaintNode* node,
  size_t            index,
  double            newX,
  double            newY,
  bool              forFrom,
  bool              effectOnFill)
{
  setPaintNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [newX, newY, forFrom](auto&& arg)
    {
      if (forFrom)
      {
        arg.from = { newX, -newY };
      }
      else
      {
        arg.to = { newX, -newY };
      }
    });
}

void AttrBridge::setGradientEllipse(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  const std::vector<double>&  value,
  bool                        effectOnFill)
{
  setLayoutNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [&value](VGG::Model::GradientInstance& ins)
    {
      if (value.size() == 1)
      {
        ins.ellipse = value[0];
      }
      else
      {
        assert(value.size() == 2);
        ins.ellipse = value;
      }
    });
}

void AttrBridge::setGradientEllipse(
  layer::PaintNode*          node,
  size_t                     index,
  const std::vector<double>& value,
  bool                       effectOnFill)
{
  setPaintNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [value](auto&& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, VGG::GradientLinear>)
      {
        assert(false);
      }
      else
      {
        if (value.size() == 1)
        {
          arg.ellipse = static_cast<float>(value[0]);
        }
        else
        {
          assert(value.size() == 2);

          glm::vec2 tmp;
          tmp[0] = static_cast<float>(value.at(0));
          tmp[1] = -static_cast<float>(value.at(1));
          arg.ellipse = tmp;
        }
      }
    });
}

void AttrBridge::setGradientStopsPosition(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  size_t                      indexForStops,
  double                      newValue,
  bool                        effectOnFill)
{
  setLayoutNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [indexForStops, newValue](VGG::Model::GradientInstance& ins)
    {
      auto size = ins.stops.size();
      if (indexForStops >= size)
      {
        assert(false);
        return;
      }

      ins.stops[indexForStops].position = newValue;
    });
}

void AttrBridge::setGradientStopsPosition(
  layer::PaintNode* node,
  size_t            index,
  size_t            indexForStops,
  double            newValue,
  bool              effectOnFill)
{
  auto size = getGradientStopsCount(node, index, effectOnFill);
  if (!size)
  {
    return;
  }

  if (indexForStops >= *size)
  {
    return;
  }

  setPaintNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [indexForStops, newValue](auto&& arg)
    { arg.stops[indexForStops].position = static_cast<float>(newValue); });
}

void AttrBridge::setGradientStopsColor(
  std::shared_ptr<LayoutNode> node,
  size_t                      index,
  size_t                      indexForStops,
  const std::vector<double>&  color,
  bool                        effectOnFill)
{
  setLayoutNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [indexForStops, &color](VGG::Model::GradientInstance& ins)
    {
      assert(color.size() == 4);

      auto size = ins.stops.size();
      if (indexForStops >= size)
      {
        assert(false);
        return;
      }

      auto& item = ins.stops[indexForStops].color;
      item.alpha = color[0];
      item.red = color[1];
      item.green = color[2];
      item.blue = color[3];
    });
}

void AttrBridge::setGradientStopsColor(
  layer::PaintNode*          node,
  size_t                     index,
  size_t                     indexForStops,
  const std::vector<double>& color,
  bool                       effectOnFill)
{
  auto size = getGradientStopsCount(node, index, effectOnFill);
  if (!size)
  {
    return;
  }

  if (indexForStops >= *size)
  {
    return;
  }

  setPaintNodeGradientAttr(
    node,
    index,
    effectOnFill,
    [indexForStops, &color](auto&& arg)
    {
      assert(color.size() == 4);
      auto& item = arg.stops[indexForStops].color;
      item.a = static_cast<float>(color[0]);
      item.r = static_cast<float>(color[1]);
      item.g = static_cast<float>(color[2]);
      item.b = static_cast<float>(color[3]);
    });
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

bool AttrBridge::checkForAccessFill(layer::PaintNode* paintNode, size_t index)
{
  if (!paintNode)
  {
    return false;
  }

  auto fillSize = AttrBridge::getFillSize(paintNode);
  assert(fillSize);

  if (index >= *fillSize)
  {
    return false;
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

std::shared_ptr<UIView> AttrBridge::getView()
{
  return m_view;
}

bool AttrBridge::addFill(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  const VGG::Model::Fill&     value,
  bool                        isOnlyUpdatePaint,
  size_t                      index)
{
  CHECK_EXPR(paintNode, false);
  GET_PAINTNODE_ACCESSOR(paintNode, accessor, false);

  auto size = *getFillSize(paintNode);
  if (index == static_cast<size_t>(-1))
  {
    index = size;
  }

  if (index > size)
  {
    return false;
  }

  // for LayoutNode
  if (!isOnlyUpdatePaint)
  {
    auto object = AttrBridge::getlayoutNodeObject(node);
    if (!object || index > object->style.fills.size())
    {
      return false;
    }
    else
    {
      auto& fills = object->style.fills;
      fills.insert(fills.begin() + index, value);
    }
  }

  // For PaintNode
  {
    auto fills = accessor->getFills();
    fills.insert(fills.begin() + index, VGG::Fill());
    VGG::layer::serde::serde_from(value, fills.at(index));

    auto id = fills.at(index).type.index();
    if (!id || 1 == id)
    {
      auto matrix = paintNode->getTransform().matrix();
      TransformHelper::changeYDirection(matrix);

      if (!id)
      {
        VGG::layer::CoordinateConvert::convertCoordinateSystem(
          std::get<VGG::Gradient>(fills.at(index).type),
          matrix);
      }
      else
      {
        VGG::layer::CoordinateConvert::convertCoordinateSystem(
          std::get<VGG::Pattern>(fills.at(index).type),
          matrix);
      }
    }

    accessor->setFills(fills);
  }

  return true;
}

bool AttrBridge::delFill(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  bool                        isOnlyUpdatePaint,
  size_t                      index)
{
  CHECK_EXPR(paintNode, false);
  GET_PAINTNODE_ACCESSOR(paintNode, accessor, false);

  auto size = *getFillSize(paintNode);
  if (index == static_cast<size_t>(-1))
  {
    index = size - 1;
  }

  if (index >= size)
  {
    return false;
  }

  // for LayoutNode
  if (!isOnlyUpdatePaint)
  {
    auto object = AttrBridge::getlayoutNodeObject(node);
    if (!object || index >= object->style.fills.size())
    {
      return false;
    }
    else
    {
      auto& fills = object->style.fills;
      fills.erase(fills.begin() + index);
    }
  }

  // For PaintNode
  {
    auto fills = accessor->getFills();
    fills.erase(fills.begin() + index);
    accessor->setFills(fills);
  }

  return true;
}

bool AttrBridge::updateFillEnabled(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  bool                           enabled,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  auto nowState = AttrBridge::getFillEnabled(paintNode, index);
  if (!nowState)
  {
    return false;
  }

  if (*nowState == enabled)
  {
    return true;
  }

  auto update = [node, paintNode, index, isOnlyUpdatePaint, enabled]()
  {
    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setFillEnabled(node, index, enabled);
    }

    AttrBridge::setFillEnabled(paintNode, index, enabled);
  };

  if (!animate)
  {
    update();
  }
  else
  {
    auto opacity = AttrBridge::getFillOpacity(paintNode, index);
    if (!opacity)
    {
      return false;
    }

    if (enabled)
    {
      updateFillOpacity(node, paintNode, index, 0, false);
      update();
      updateFillOpacity(node, paintNode, index, *opacity, false, animate);
    }
    else
    {
      updateFillOpacity(node, paintNode, index, 0, false, animate);
      animate->addCallBackWhenStop(
        [this, node, paintNode, index, opacity, update]()
        {
          update();
          updateFillOpacity(node, paintNode, index, *opacity, false);
        });
    }
  }

  return true;
}

bool AttrBridge::updateFillColor(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  const Model::Color&            newColor,
  bool                           isOnlyUpdatePaint,
  std::shared_ptr<NumberAnimate> animate)
{
  auto nowColor = getFillColor(paintNode, index);
  if (!nowColor)
  {
    return false;
  }

  if (index >= *AttrBridge::getFillSize(paintNode))
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

  updateSimpleAttr(
    { nowColor->a, nowColor->r, nowColor->g, nowColor->b },
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
  if (!checkForAccessFill(paintNode, index))
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

bool AttrBridge::updateFillBlendMode(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  size_t                      index,
  int                         newBlendMode,
  bool                        isOnlyUpdatePaint)
{
  // 27: Pass through
  if (newBlendMode == 27)
  {
    return false;
  }

  if (!checkForAccessFill(paintNode, index))
  {
    return false;
  }

  auto blendMode = g_blendMode.find(newBlendMode);
  if (blendMode == g_blendMode.end())
  {
    return false;
  }

  if (!isOnlyUpdatePaint)
  {
    AttrBridge::setFillBlendMode(node, index, newBlendMode);
  }

  AttrBridge::setFillBlendMode(paintNode, index, blendMode->second);
  return true;
}

bool AttrBridge::updatePatternImageFilters(
  std::shared_ptr<LayoutNode>        node,
  layer::PaintNode*                  paintNode,
  size_t                             index,
  std::optional<Model::ImageFilters> imageFilters,
  bool                               isOnlyUpdatePaint,
  bool                               effectOnFill,
  std::shared_ptr<NumberAnimate>     animate)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    auto update = [node, paintNode, index, isOnlyUpdatePaint](const std::vector<double>& value)
    {
      assert(value.size() == 8);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setPatternImageFilters(node, index, value, true);
      }

      AttrBridge::setPatternImageFilters(paintNode, index, value, true);
    };

    auto                nowFilter = *getPatternImageFilters(paintNode, index, true);
    std::vector<double> oldValue{ nowFilter.exposure,    nowFilter.contrast, nowFilter.saturation,
                                  nowFilter.temperature, nowFilter.tint,     nowFilter.highlight,
                                  nowFilter.shadow,      nowFilter.hue };

    std::vector<double> newValue(8);
    if (imageFilters && imageFilters->isEnabled)
    {
      int i = 0;
      newValue.at(i++) = imageFilters->exposure ? *imageFilters->exposure : 0;
      newValue.at(i++) = imageFilters->contrast ? *imageFilters->contrast : 0;
      newValue.at(i++) = imageFilters->saturation ? *imageFilters->saturation : 0;
      newValue.at(i++) = imageFilters->temperature ? *imageFilters->temperature : 0;
      newValue.at(i++) = imageFilters->tint ? *imageFilters->tint : 0;
      newValue.at(i++) = imageFilters->highlights ? *imageFilters->highlights : 0;
      newValue.at(i++) = imageFilters->shadows ? *imageFilters->shadows : 0;
      newValue.at(i++) = imageFilters->hue ? *imageFilters->hue : 0;
    }

    updateSimpleAttr(oldValue, newValue, update, animate);
    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updatePatternImageFillRotation(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  double                         newRotation,
  bool                           isOnlyUpdatePaint,
  bool                           effectOnFill,
  std::shared_ptr<NumberAnimate> animate)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    auto update = [node, paintNode, index, isOnlyUpdatePaint](const std::vector<double>& value)
    {
      assert(value.size() == 1);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setPatternRotation(node, index, value.at(0), true);
      }

      AttrBridge::setPatternRotation(paintNode, index, value.at(0), true);
    };

    updateSimpleAttr(
      { *getPatternRotation(paintNode, index, true) },
      { newRotation },
      update,
      animate);
    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updatePatternImageTileMirror(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  size_t                      index,
  bool                        newMirror,
  bool                        isOnlyUpdatePaint,
  bool                        effectOnFill)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setPatternImageTileMirror(node, index, newMirror, true);
    }

    AttrBridge::setPatternImageTileMirror(paintNode, index, newMirror, true);

    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updatePatternImageTileScale(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  double                         newScale,
  bool                           isOnlyUpdatePaint,
  bool                           effectOnFill,
  std::shared_ptr<NumberAnimate> animate)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    auto update = [node, paintNode, index, isOnlyUpdatePaint](const std::vector<double>& value)
    {
      assert(value.size() == 1);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setPatternImageTileScale(node, index, value.at(0), true);
      }

      AttrBridge::setPatternImageTileScale(paintNode, index, value.at(0), true);
    };

    updateSimpleAttr(
      { *getPatternImageTileScale(paintNode, index, true) },
      { newScale },
      update,
      animate);
    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updatePatternImageTileMode(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  size_t                      index,
  int                         newMode,
  bool                        isOnlyUpdatePaint,
  bool                        effectOnFill)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setPatternImageTileMode(node, index, newMode, true);
    }

    AttrBridge::setPatternImageTileMode(paintNode, index, newMode, true);

    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updateGradientFromOrTo(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  double                         newX,
  double                         newY,
  bool                           isOnlyUpdatePaint,
  bool                           forFrom,
  bool                           effectOnFill,
  std::shared_ptr<NumberAnimate> animate)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    auto update =
      [node, paintNode, index, isOnlyUpdatePaint, forFrom](const std::vector<double>& value)
    {
      assert(value.size() == 2);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setGradientFromOrTo(node, index, value[0], value[1], forFrom, true);
      }

      AttrBridge::setGradientFromOrTo(paintNode, index, value[0], value[1], forFrom, true);
    };

    auto oldValue = *getGradientFromOrTo(paintNode, index, forFrom, true);

    updateSimpleAttr({ oldValue[0], oldValue[1] }, { newX, newY }, update, animate);
    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updateGradientEllipse(
  std::shared_ptr<LayoutNode>                 node,
  layer::PaintNode*                           paintNode,
  size_t                                      index,
  std::variant<double, std::array<double, 2>> newEllipse,
  bool                                        isOnlyUpdatePaint,
  bool                                        effectOnFill,
  std::shared_ptr<NumberAnimate>              animate)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    auto update = [node, paintNode, index, isOnlyUpdatePaint](const std::vector<double>& value)
    {
      assert(value.size() == 1 || value.size() == 2);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setGradientEllipse(node, index, value, true);
      }

      AttrBridge::setGradientEllipse(paintNode, index, value, true);
    };

    auto oldValue = getGradientEllipse(paintNode, index, true);
    if (!oldValue)
    {
      return false;
    }

    if (oldValue->index() != newEllipse.index())
    {
      animate = nullptr;
    }

    if (!oldValue->index())
    {
      updateSimpleAttr(
        { std::get<double>(*oldValue) },
        { std::get<double>(newEllipse) },
        update,
        animate);
    }
    else
    {
      const auto& v0 = std::get<std::array<double, 2>>(*oldValue);
      const auto& v1 = std::get<std::array<double, 2>>(newEllipse);

      updateSimpleAttr({ v0[0], v0[1] }, { v1[0], v1[1] }, update, animate);
    }

    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updateGradientStopPosition(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  size_t                         indexForStops,
  double                         newValue,
  bool                           isOnlyUpdatePaint,
  bool                           effectOnFill,
  std::shared_ptr<NumberAnimate> animate)
{
  if (effectOnFill)
  {
    if (
      !checkForAccessFill(paintNode, index) ||
      indexForStops >= *getGradientStopsCount(paintNode, index, true))
    {
      return false;
    }

    auto update =
      [node, paintNode, index, isOnlyUpdatePaint, indexForStops](const std::vector<double>& value)
    {
      assert(value.size() == 1);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setGradientStopsPosition(node, index, indexForStops, value[0], true);
      }

      AttrBridge::setGradientStopsPosition(paintNode, index, indexForStops, value[0], true);
    };

    auto oldValue = *getGradientStopsPosition(paintNode, index, indexForStops, true);

    updateSimpleAttr({ oldValue }, { newValue }, update, animate);
    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::updateGradientStopColor(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
  size_t                         index,
  size_t                         indexForStops,
  const VGG::Model::Color&       newColor,
  bool                           isOnlyUpdatePaint,
  bool                           effectOnFill,
  std::shared_ptr<NumberAnimate> animate)
{
  if (effectOnFill)
  {
    if (
      !checkForAccessFill(paintNode, index) ||
      indexForStops >= *getGradientStopsCount(paintNode, index, true))
    {
      return false;
    }

    auto nowColor = getGradientStopsColor(paintNode, index, indexForStops, true);
    if (!nowColor)
    {
      return false;
    }

    auto update =
      [node, paintNode, index, isOnlyUpdatePaint, indexForStops](const std::vector<double>& value)
    {
      assert(value.size() == 4);

      if (!isOnlyUpdatePaint)
      {
        AttrBridge::setGradientStopsColor(node, index, indexForStops, value, true);
      }

      AttrBridge::setGradientStopsColor(paintNode, index, indexForStops, value, true);
    };

    updateSimpleAttr(
      { nowColor->a, nowColor->r, nowColor->g, nowColor->b },
      { newColor.alpha, newColor.red, newColor.green, newColor.blue },
      update,
      animate);

    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::delGradientStop(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  size_t                      index,
  size_t                      indexForStops,
  bool                        isOnlyUpdatePaint,
  bool                        effectOnFill)
{
  if (effectOnFill)
  {
    if (
      !checkForAccessFill(paintNode, index) ||
      indexForStops >= *getGradientStopsCount(paintNode, index, true))
    {
      return false;
    }

    if (!isOnlyUpdatePaint)
    {
      setLayoutNodeGradientAttr(
        node,
        index,
        effectOnFill,
        [indexForStops](VGG::Model::GradientInstance& ins)
        {
          auto size = ins.stops.size();
          if (indexForStops >= size)
          {
            assert(false);
            return;
          }

          ins.stops.erase(ins.stops.begin() + indexForStops);
        });
    }

    setPaintNodeGradientAttr(
      paintNode,
      index,
      effectOnFill,
      [indexForStops](auto&& arg) { arg.stops.erase(arg.stops.begin() + indexForStops); });

    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
}

bool AttrBridge::addGradientStop(
  std::shared_ptr<LayoutNode>     node,
  layer::PaintNode*               paintNode,
  size_t                          index,
  size_t                          indexForStops,
  const VGG::Model::GradientStop& gradientStop,
  bool                            isOnlyUpdatePaint,
  bool                            effectOnFill)
{
  if (effectOnFill)
  {
    if (
      !checkForAccessFill(paintNode, index) ||
      indexForStops > *getGradientStopsCount(paintNode, index, true))
    {
      return false;
    }

    if (!isOnlyUpdatePaint)
    {
      setLayoutNodeGradientAttr(
        node,
        index,
        effectOnFill,
        [indexForStops, &gradientStop](VGG::Model::GradientInstance& ins)
        {
          auto size = ins.stops.size();
          if (indexForStops > size)
          {
            assert(false);
            return;
          }

          ins.stops.emplace(ins.stops.begin() + indexForStops, gradientStop);
        });
    }

    VGG::GradientStop value;
    value.color.a = static_cast<float>(gradientStop.color.alpha);
    value.color.r = static_cast<float>(gradientStop.color.red);
    value.color.g = static_cast<float>(gradientStop.color.green);
    value.color.b = static_cast<float>(gradientStop.color.blue);
    value.midPoint = static_cast<float>(gradientStop.midPoint);
    value.position = static_cast<float>(gradientStop.position);
    setPaintNodeGradientAttr(
      paintNode,
      index,
      effectOnFill,
      [indexForStops, &value](auto&& arg)
      { arg.stops.emplace(arg.stops.begin() + indexForStops, std::move(value)); });

    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }

  return false;
}

bool AttrBridge::updatePatternImageFileName(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  size_t                      index,
  std::string                 newName,
  bool                        isOnlyUpdatePaint,
  bool                        effectOnFill)
{
  if (effectOnFill)
  {
    if (!checkForAccessFill(paintNode, index))
    {
      return false;
    }

    if (!isOnlyUpdatePaint)
    {
      AttrBridge::setPatternImageFileName(node, index, newName, true);
    }

    AttrBridge::setPatternImageFileName(paintNode, index, newName, true);

    return true;
  }
  else
  {
    // TODO not complete.

    return false;
  }
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
  auto matrix =
    TransformHelper::fromDesignMatrix(TransformHelper::translate(x, y, { 1, 0, 0, 1, 0, 0 }));
  node->setContentTransform(layer::Transform(matrix));
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
#ifndef NDEBUG
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
#endif // !NDEBUG

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

    parent->addChild(++it, layer::incRef(result.root->at(0)->node()));

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
    animate->addCallBackWhenStop([this, paintNode, index]() { delChild(paintNode, index); });
  }

  return true;
}

bool AttrBridge::scrollTo(
  std::shared_ptr<LayoutNode>    node,
  layer::PaintNode*              paintNode,
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

  auto oldValue = getScrollInfo(paintNode);
  updateSimpleAttr(
    { (*oldValue)[0], (*oldValue)[1] },
    { newValue[0], newValue[1] },
    update,
    animate);
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
