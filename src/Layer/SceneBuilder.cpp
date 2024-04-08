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

#include "Layer/Core/VShape.hpp"
#include "Layer/DocConcept.hpp"
#include "Layer/JSONModel.hpp"
#include "Layer/Memory/VAllocator.hpp"
#include "PathPatch.h"
#include "Layer/SceneBuilder.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/CoordinateConvert.hpp"
#include "Layer/ParagraphParser.hpp"
#include "AttrSerde.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/ImageNode.hpp"
#include <concepts>
#include <variant>

#include "Layer/JSONModel.hpp"

namespace
{

using namespace VGG::layer;
struct BuilderImpl
{

  static json defaultTextAttr()
  {
    auto j = R"({
        "length":0,
        "name":"Fira Sans",
        "subFamilyName":"",
        "size":14,
        "fontVariations":[],
        "postScript":"",
        "kerning":true,
        "letterSpacingValue":0,
        "letterSpacingUnit":0,
        "lineSpacingValue":0,
        "lineSpacingUnit":0,
        "fillUseType":0,
        "underline":0,
        "linethrough":false,
        "fontVariantCaps":0,
        "textCase":0,
        "baselineShift":0,
        "baseline":0,
        "horizontalScale":1,
        "verticalScale":1,
        "proportionalSpacing":0,
        "rotate":0,
        "textParagraph":{}
    })"_json;
    return j;
  }
  static std::tuple<glm::mat3, glm::mat3, glm::mat3> makeMatrix(const glm::mat3& m)
  {
    const auto [newMatrix, inversed] = CoordinateConvert::convertMatrixCoordinate(m);
    return { m, newMatrix, inversed };
  }

  static Bounds makeBounds(const Bounds& bounds, const glm::mat3& totalMatrix)
  {
    auto x = bounds.topLeft().x;
    auto y = bounds.topLeft().y;
    auto width = bounds.width();
    auto height = bounds.height();
    auto topLeft = glm::vec2{ x, y };
    auto bottomRight = glm::vec2{ x + width, y - height };
    CoordinateConvert::convertCoordinateSystem(topLeft, totalMatrix);
    CoordinateConvert::convertCoordinateSystem(bottomRight, totalMatrix);
    return Bounds{ topLeft, bottomRight };
  }

  template<typename T, typename F1, typename F2>
    requires AbstractObject<T> && CallableObject<PaintNode*, F1, std::string, std::string> &&
             CallableObject<void, F2, PaintNode*, glm::mat3, Bounds>
  static PaintNodePtr makeObjectBase(
    const T&         m,
    const glm::mat3& totalMatrix,
    F1&&             creator,
    F2&&             override)
  {
    auto obj = creator(m.getName(), m.getId());
    if (!obj)
      return nullptr;
    Bounds bounds = m.getBounds();
    auto   style = m.getStyle();

    auto [originalMatrix, newMatrix, inversedNewMatrix] = BuilderImpl::makeMatrix(m.getMatrix());
    const auto convertedMatrix = inversedNewMatrix * totalMatrix * originalMatrix;
    CoordinateConvert::convertCoordinateSystem(bounds, totalMatrix);
    CoordinateConvert::convertCoordinateSystem(style, totalMatrix);

    obj->setTransform(Transform(newMatrix));
    obj->setFrameBounds(bounds);
    obj->setStyle(style);
    obj->setFrameCornerSmoothing(m.getCornerSmoothing());
    obj->setContextSettings(m.getContextSetting());
    obj->setMaskBy(m.getShapeMask());
    obj->setAlphaMaskBy(m.getAlphaMask());

    const auto maskType = m.getMaskType();
    obj->setMaskType(maskType);
    // const auto defaultShowType =
    //   maskType == EMaskType::MT_OUTLINE
    //     ? MST_CONTENT
    //     : (maskType == EMaskType::MT_ALPHA && false ? MST_BOUNDS : MST_INVISIBLE);
    obj->setMaskShowType(m.getMaskShowType()); // default show type??
    obj->setOverflow(m.getOverflow());
    obj->setVisible(m.getVisible());
    override(0, convertedMatrix, Bounds());
    return obj;
  }

  template<typename T, typename C>
    requires layer::GroupObject<T> && layer::CastObject<C, T>
  static PaintNodePtr fromGroup(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(alloc, std::move(name), VGG_GROUP, std::move(guid));
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bound)
      {
        p->setOverflow(OF_VISIBLE); // Group do not clip inner content
        p->setContourOption(ContourOption(ECoutourType::MCT_UNION, false));
        p->setPaintOption(EPaintStrategy(EPaintStrategy::PS_CHILDONLY));
        auto childObjects = m.getChildObjects();
        for (const auto& c : childObjects)
        {
          p->addChild(BuilderImpl::fromObject<T, C>(C::asGroup(c), matrix, alloc));
        }
      });
  }

  template<typename T, typename C>
    requires layer::FrameObject<T> && layer::CastObject<C, T>
  static PaintNodePtr fromFrame(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(alloc, std::move(name), VGG_FRAME, std::move(guid));
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bound)
      {
        p->setContourOption(ContourOption(ECoutourType::MCT_FRAMEONLY, false));
        p->setFrameRadius(m.getRadius());
        auto childObjects = m.getChildObjects();
        for (const auto& c : childObjects)
        {
          p->addChild(BuilderImpl::fromObject<T, C>(C::asFrame(c), matrix, alloc));
        }
      });
  }
  template<typename T, typename C>
    requires layer::PathObject<T> && layer::CastObject<C, T>
  static PaintNodePtr fromPath(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(alloc, std::move(name), VGG_PATH, std::move(guid));
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
      {
        p->setChildWindingType(m.getWindingType());
        p->setContourOption(ContourOption(ECoutourType::MCT_OBJECT_OPS, false));
        p->setPaintOption(PaintOption(EPaintStrategy::PS_SELFONLY));
        const auto shapes = m.getShapes();
        for (const auto& subshape : shapes)
        {
          const auto blop = subshape.booleanOperation;
          const auto geo = subshape.geometry;
#define VT(type)                                                                                   \
  auto ptr = std::get_if<type>(&geo);                                                              \
  ptr
          if (VT(typename T::BaseType))
          {
            // none shape child
            p->addSubShape(fromObject<typename T::BaseType, C>(*ptr, totalMatrix, alloc), blop);
          }
          else if (VT(ShapeData))
          {
            auto node = makePaintNodePtr(alloc, "contour", VGG_CONTOUR, "");
            node->setOverflow(OF_VISIBLE);
            node->setContourOption(ContourOption{ ECoutourType::MCT_FRAMEONLY, false });
            // node->setContourData(makeShapeData(geo, j, matrix)); // TODO
            p->addSubShape(node, blop);
          }
#undef VT
        }
      });
  }

  template<typename T, typename C>
    requires layer::FrameObject<T> && layer::CastObject<C, T>
  static std::vector<PaintNodePtr> fromTopLevelFrames(
    const std::vector<T>& frameObject,
    const glm::mat3&      matrix,
    VAllocator*           alloc)
  {
    std::vector<PaintNodePtr> frames(4);
    for (const auto& f : frameObject)
    {
      frames.push_back(BuilderImpl::fromFrame<T, C>(f, matrix, alloc));
    }
    return frames;
  }

  template<typename T, typename C>
    requires layer::MasterObject<T> && layer::CastObject<C, T>
  static PaintNodePtr fromMaster(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(alloc, std::move(name), VGG_FRAME, std::move(guid));
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bound)
      {
        p->setContourOption(ContourOption(ECoutourType::MCT_FRAMEONLY, false));
        p->setFrameRadius(m.getRadius());
        auto childObjects = m.getChildObjects();
        for (const auto& c : childObjects)
        {
          p->addChild(BuilderImpl::fromObject<T, C>(C::asMaster(c), matrix, alloc));
        }
      });
  }

  template<typename T, typename C>
    requires layer::InstanceObject<T> && layer::CastObject<C, T>
  static PaintNodePtr fromInstance(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    DEBUG("not support instance");
    return nullptr;
  }

  template<typename T, typename C>
    requires layer::ImageObject<T> && layer::CastObject<C, T>
  static PaintNodePtr fromImage(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      { return makeImageNodePtr(alloc, std::move(name), std::move(guid)); },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
      {
        auto i = static_cast<ImageNode*>(p);
        i->setImageBounds(bounds);
        i->setImage(m.getImageGUID());
        i->setImageFilter(m.getImageFilter());
        // i->setReplacesImage(j.value("fillReplacesImage", false));
      });
  }

  template<typename T, typename C>
    requires layer::TextObject<T> && layer::CastObject<C, T>
  static PaintNodePtr fromText(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      { return makeTextNodePtr(alloc, std::move(name), std::move(guid)); },
      [&](PaintNode* ptr, const glm::mat3& matrix, const Bounds& bounds)
      {
        auto p = static_cast<TextNode*>(ptr);
        p->setVerticalAlignment(m.getVerticalAlignment());
        p->setFrameMode(m.getLayoutMode());
        if (auto anchor = m.getAnchor(); anchor)
        {
          glm::vec2 anchorPoint = { (*anchor)[0], (*anchor)[1] };
          CoordinateConvert::convertCoordinateSystem(anchorPoint, totalMatrix);
          p->setTextAnchor(anchorPoint);
        }
        p->setParagraphBounds(bounds);

        // 1.

        std::vector<TextStyleAttr> textStyle;
        // auto                       defaultAttr = defaultTextAttr();
        // defaultAttr.update(j.value("defaultFontAttr", json::object()), true);
        // auto fontAttr = j.value("fontAttr", std::vector<json>{});
        // for (auto& att : fontAttr)
        // {
        //   auto json = defaultAttr;
        //   json.update(att, true);
        //   if (auto it = json.find("fills"); it == json.end())
        //   {
        //     json["fills"] =
        //       j.value("style", nlohmann::json{}).value("fills", std::vector<nlohmann::json>());
        //   }
        //   if (auto it = json.find("borders"); it == json.end())
        //   {
        //
        //     json["borders"] =
        //       j.value("style", nlohmann::json{}).value("borders", std::vector<nlohmann::json>());
        //   }
        //   textStyle.push_back(json);
        // }

        for (auto& style : textStyle)
        {
          CoordinateConvert::convertCoordinateSystem(style, totalMatrix);
        }

        // 2.
        auto lineType = m.getTextLineType();
        auto alignments = m.getHorizontalAlignment();

        std::vector<ParagraphAttr> parStyle;
        parStyle.reserve(lineType.size());

        size_t       i = 0;
        auto         defaultAlign = alignments.empty() ? HA_LEFT : alignments.back();
        TextLineAttr defaultLineType;
        while (i < lineType.size() && i < alignments.size())
        {
          parStyle.emplace_back(lineType[i], alignments[i]);
          i++;
        }
        while (i < lineType.size())
        {
          parStyle.emplace_back(lineType[i], defaultAlign);
          i++;
        }
        while (i < alignments.size())
        {
          parStyle.emplace_back(defaultLineType, alignments[i]);
          i++;
        }
        // if (m_fontNameVisitor)
        // {
        //   for (const auto& style : textStyle)
        //   {
        //     m_fontNameVisitor(style.font.fontName, style.font.subFamilyName);
        //   }
        // }
        p->setParagraph(m.getText(), std::move(textStyle), std::move(parStyle));
        if (bounds.width() == 0 || bounds.height() == 0)
        {
          p->setFrameMode(TL_AUTOWIDTH);
        }
        return p;
      });
  }

  template<typename T, typename C>
    requires layer::AbstractObject<T> && CastObject<C, T>
  static PaintNodePtr fromObject(const T& m, const glm::mat3& totalMatrix, VAllocator* alloc)
  {
    PaintNodePtr ro;
    switch (m.getObjectType())
    {
      case EModelObjectType::GROUP:
        return BuilderImpl::fromGroup<decltype(C::asGroup(m)), C>(
          C::asGroup(m),
          totalMatrix,
          alloc);
      case EModelObjectType::FRAME:
        return BuilderImpl::fromFrame<decltype(C::asFrame(m)), C>(
          C::asFrame(m),
          totalMatrix,
          alloc);
      case EModelObjectType::PATH:
        return BuilderImpl::fromPath<decltype(C::asPath(m)), C>(C::asPath(m), totalMatrix, alloc);
      case EModelObjectType::IMAGE:
        return BuilderImpl::fromImage<decltype(C::asImage(m)), C>(
          C::asImage(m),
          totalMatrix,
          alloc);
      case EModelObjectType::TEXT:
        return BuilderImpl::fromText<decltype(C::asText(m)), C>(C::asText(m), totalMatrix, alloc);
      case EModelObjectType::MASTER:
        return BuilderImpl::fromMaster<decltype(C::asMaster(m)), C>(
          C::asMaster(m),
          totalMatrix,
          alloc);
        break;
      case EModelObjectType::INSTANCE:
        return BuilderImpl::fromInstance<decltype(C::asInstance(m)), C>(
          C::asInstance(m),
          totalMatrix,
          alloc);
      case EModelObjectType::CONTOUR:
        DEBUG("not reachable");
        break;
      case EModelObjectType::UNKNOWN:
        DEBUG("unknown object type");
        break;
    }
    return nullptr;
  }
};

} // namespace

namespace
{
using namespace VGG::layer;
PaintNodePtr makeContourNode(VAllocator* alloc)
{
  auto p = makePaintNodePtr(alloc, "contour", VGG_CONTOUR, "");
  p->setOverflow(OF_VISIBLE);
  p->setContourOption(ContourOption{ ECoutourType::MCT_FRAMEONLY, false });
  return p;
}

ContourPtr makeContourData(const json& j, const json& parent, const glm::mat3& totalMatrix)
{
  Contour contour;
  contour.closed = j.value("closed", false);
  const auto& points = getOrDefault(j, "points");
  for (const auto& e : points)
  {
    contour.emplace_back(
      getOptional<glm::vec2>(e, "point").value_or(glm::vec2{ 0, 0 }),
      getOptional<float>(e, "radius").value_or(0.0),
      getOptional<glm::vec2>(e, "curveFrom"),
      getOptional<glm::vec2>(e, "curveTo"),
      getOptional<int>(e, "cornerStyle"));
  }
  CoordinateConvert::convertCoordinateSystem(contour, totalMatrix);
  auto ptr = std::make_shared<Contour>(contour);
  ptr->cornerSmooth = getOptional<float>(parent, "cornerSmoothing").value_or(0.f);
  if (!ptr->closed && !ptr->empty())
  {
    ptr->back().radius = 0;
    ptr->front().radius = 0;
  }
  return ptr;
}

ShapeData makeShapeData(const json& j, const json& parent, const glm::mat3& totalMatrix)
{
  const auto klass = j.value("class", "");
  const auto rect = SkRect::MakeXYWH(
    parent["bounds"]["x"].get<float>(),
    -parent["bounds"]["y"].get<float>(),
    parent["bounds"]["width"].get<float>(),
    parent["bounds"]["height"].get<float>());
  if (klass == "contour")
  {
    return makeContourData(j, parent, totalMatrix);
  }
  else if (klass == "rectangle")
  {
    const auto rect = SkRect::MakeXYWH(
      parent["bounds"]["x"].get<float>(),
      -parent["bounds"]["y"].get<float>(),
      parent["bounds"]["width"].get<float>(),
      parent["bounds"]["height"].get<float>());
    std::array<float, 4> radius = j.value("radius", std::array<float, 4>{ 0, 0, 0, 0 });
    auto cornerSmoothing = getOptional<float>(parent, "cornerSmoothing").value_or(0.f);
    auto s = makeShape(radius, rect, cornerSmoothing);
    return std::visit([&](auto&& arg) { return ShapeData(arg); }, s);
  }
  else if (klass == "ellipse")
  {
    Ellipse oval;
    oval.rect = rect;
    return oval;
  }
  else if (klass == "polygon" || klass == "star")
  {
    auto cp = parent;
    if (pathChange(cp))
    {
      return makeContourData(cp["shape"]["subshapes"][0]["subGeometry"], cp, totalMatrix);
    }
  }
  return ShapeData();
}

template<typename F1, typename F2>
inline PaintNodePtr makeObjectCommonProperty(
  const json&      j,
  const glm::mat3& totalMatrix,
  F1&&             creator,
  F2&&             override)
{
  auto obj = creator(j.value("name", ""), j.value("id", ""));
  if (!obj)
    return nullptr;
  auto [originalMatrix, newMatrix, inversedNewMatrix] =
    SceneBuilder::fromMatrix(j.value("matrix", json::array_t{}));
  const auto convertedMatrix = inversedNewMatrix * totalMatrix * originalMatrix;

  obj->setTransform(Transform(newMatrix));
  const auto b = SceneBuilder::fromBounds(j.value("bounds", json::object_t{}), convertedMatrix);
  obj->setFrameBounds(b);

  // Pattern point in style are implicitly given by bounds, we must supply the points in original
  // coordinates for correct converting
  obj->setStyle(SceneBuilder::fromStyle(j.value("style", json::object_t{}), b, convertedMatrix));
  obj->setFrameCornerSmoothing(getOptional<float>(j, "cornerSmoothing").value_or(0.f));
  obj->setContextSettings(j.value("contextSettings", ContextSetting()));
  obj->setMaskBy(j.value("outlineMaskBy", std::vector<std::string>{}));
  obj->setAlphaMaskBy(j.value("alphaMaskBy", std::vector<AlphaMask>{}));
  const auto maskType = j.value("maskType", EMaskType::MT_NONE);
  const auto defaultShowType =
    maskType == EMaskType::MT_OUTLINE
      ? MST_CONTENT
      : (maskType == EMaskType::MT_ALPHA && false ? MST_BOUNDS : MST_INVISIBLE);
  const auto maskShowType = j.value("maskShowType", defaultShowType);
  obj->setMaskType(maskType);
  obj->setMaskShowType(maskShowType);
  obj->setOverflow(j.value("overflow", EOverflow::OF_VISIBLE));
  obj->setVisible(j.value("visible", true));
  override(obj.get(), convertedMatrix, b);
  return obj;
}

} // namespace

namespace VGG::layer
{
using namespace nlohmann;

json SceneBuilder::defaultTextAttr()
{
  auto j = R"({
        "length":0,
        "name":"Fira Sans",
        "subFamilyName":"",
        "size":14,
        "fontVariations":[],
        "postScript":"",
        "kerning":true,
        "letterSpacingValue":0,
        "letterSpacingUnit":0,
        "lineSpacingValue":0,
        "lineSpacingUnit":0,
        "fillUseType":0,
        "underline":0,
        "linethrough":false,
        "fontVariantCaps":0,
        "textCase":0,
        "baselineShift":0,
        "baseline":0,
        "horizontalScale":1,
        "verticalScale":1,
        "proportionalSpacing":0,
        "rotate":0,
        "textParagraph":{}
    })"_json;
  return j;
}

std::tuple<glm::mat3, glm::mat3, glm::mat3> SceneBuilder::fromMatrix(const json& j)
{
  std::array<float, 6> v = j;
  auto                 original =
    glm::mat3{ glm::vec3{ v[0], v[1], 0 }, glm::vec3{ v[2], v[3], 0 }, glm::vec3{ v[4], v[5], 1 } };
  const auto [newMatrix, inversed] = CoordinateConvert::convertMatrixCoordinate(original);
  return { original, newMatrix, inversed };
}

Bounds SceneBuilder::fromBounds(const json& j, const glm::mat3& totalMatrix)
{
  auto x = j.value("x", 0.f);
  auto y = j.value("y", 0.f);
  auto width = j.value("width", 0.f);
  auto height = j.value("height", 0.f);
  auto topLeft = glm::vec2{ x, y };
  auto bottomRight = glm::vec2{ x + width, y - height };
  CoordinateConvert::convertCoordinateSystem(topLeft, totalMatrix);
  CoordinateConvert::convertCoordinateSystem(bottomRight, totalMatrix);
  return Bounds{ topLeft, bottomRight };
}

Style SceneBuilder::fromStyle(const json& j, const Bounds& bounds, const glm::mat3& totalMatrix)
{
  Style style;
  from_json(j, style);
  CoordinateConvert::convertCoordinateSystem(style, totalMatrix);
  return style;
}

PaintNodePtr SceneBuilder::fromObject(const json& j, const glm::mat3& totalMatrix)
{
  PaintNodePtr ro;
  auto         klass = j.value("class", "");
  if (klass == "group")
  {
    ro = fromGroup(j, totalMatrix);
  }
  else if (klass == "path")
  {
    ro = fromPath(j, totalMatrix);
  }
  else if (klass == "image")
  {
    ro = fromImage(j, totalMatrix);
  }
  else if (klass == "text")
  {
    ro = fromText(j, totalMatrix);
  }
  else if (klass == "symbolInstance")
  {
    ro = fromSymbolInstance(j, totalMatrix);
  }
  else if (klass == "frame")
  {
    ro = fromFrame(j, totalMatrix);
  }
  else if (klass == "symbolMaster")
  {
    ro = fromSymbolMaster(j, totalMatrix);
  }
  else
  {
    // error
    return nullptr;
  }
  return ro;
}

inline PaintNodePtr SceneBuilder::fromImage(const json& j, const glm::mat3& totalMatrix)
{
  return makeObjectCommonProperty(
    j,
    totalMatrix,
    [&](std::string name, std::string guid)
    {
      auto p = makeImageNodePtr(m_alloc, j.value("name", ""), std::move(guid));
      return p;
    },
    [&](ImageNode* p, const glm::mat3& matrix, const Bounds& bounds)
    {
      p->setImageBounds(bounds);
      p->setImage(j.value("imageFileName", ""));
      // p->setReplacesImage(j.value("fillReplacesImage", false));
      p->setImageFilter(j.value("imageFilters", ImageFilter()));
    });
}

PaintNodePtr SceneBuilder::fromPath(const json& j, const glm::mat3& totalMatrix)
{
  return makeObjectCommonProperty(
    j,
    totalMatrix,
    [&](std::string name, std::string guid)
    {
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_PATH, std::move(guid));
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
    {
      const auto shape = j.value("shape", json{});
      p->setChildWindingType(shape.value("windingRule", EWindingType::WR_EVEN_ODD));
      p->setContourOption(ContourOption(ECoutourType::MCT_OBJECT_OPS, false));
      p->setPaintOption(PaintOption(EPaintStrategy::PS_SELFONLY));
      const auto shapes = shape.value("subshapes", std::vector<json>{});
      for (const auto& subshape : shapes)
      {
        const auto blop = subshape.value("booleanOperation", EBoolOp::BO_NONE);
        const auto geo = subshape.value("subGeometry", nlohmann::json{});
        const auto klass = geo.value("class", "");
        if (
          klass == "contour" || klass == "rectangle" || klass == "ellipse" || klass == "polygon" ||
          klass == "star")
        {
          auto shape = makeContourNode(m_alloc);
          shape->setContourData(makeShapeData(geo, j, matrix));
          p->addSubShape(shape, blop);
        }
        else if (klass == "path")
        {
          p->addSubShape(fromPath(geo, matrix), blop);
        }
        else if (klass == "image")
        {
          p->addSubShape(fromImage(geo, matrix), blop);
        }
        else if (klass == "text")
        {
          p->addSubShape(fromText(geo, matrix), blop);
        }
        else if (klass == "group")
        {
          p->addSubShape(fromGroup(geo, matrix), blop);
        }
        else if (klass == "symbolInstance")
        {
          p->addSubShape(fromSymbolInstance(geo, matrix), blop);
        }
        else if (klass == "frame")
        {
          p->addSubShape(fromFrame(geo, matrix), blop);
        }
        else if (klass == "symbolMaster")
        {
          p->addSubShape(fromSymbolMaster(geo, matrix), blop);
        }
      }
    });
}

PaintNodePtr SceneBuilder::fromText(const json& j, const glm::mat3& totalMatrix)
{
  return makeObjectCommonProperty(
    j,
    totalMatrix,
    [&](std::string name, std::string guid)
    {
      auto p = makeTextNodePtr(m_alloc, std::move(name), std::move(guid));
      return p;
    },
    [&](layer::TextNode* p, const glm::mat3& matrix, const Bounds& bounds)
    {
      p->setVerticalAlignment(j.value("verticalAlignment", ETextVerticalAlignment::VA_TOP));
      p->setFrameMode(j.value("frameMode", ETextLayoutMode::TL_FIXED));
      if (auto anchor = getStackOptional<std::array<float, 2>>(j, "anchorPoint"); anchor)
      {
        glm::vec2 anchorPoint = { (*anchor)[0], (*anchor)[1] };
        CoordinateConvert::convertCoordinateSystem(anchorPoint, totalMatrix);
        p->setTextAnchor(anchorPoint);
      }
      p->setParagraphBounds(bounds);

      // 1.

      std::vector<TextStyleAttr> textStyle;

      auto defaultAttr = defaultTextAttr();
      defaultAttr.update(j.value("defaultFontAttr", json::object()), true);
      auto fontAttr = j.value("fontAttr", std::vector<json>{});
      for (auto& att : fontAttr)
      {
        auto json = defaultAttr;
        json.update(att, true);
        if (auto it = json.find("fills"); it == json.end())
        {
          json["fills"] =
            j.value("style", nlohmann::json{}).value("fills", std::vector<nlohmann::json>());
        }
        if (auto it = json.find("borders"); it == json.end())
        {

          json["borders"] =
            j.value("style", nlohmann::json{}).value("borders", std::vector<nlohmann::json>());
        }
        textStyle.push_back(json);
      }

      for (auto& style : textStyle)
      {
        CoordinateConvert::convertCoordinateSystem(style, totalMatrix);
      }

      // 2.
      auto lineType = j.value("textLineType", std::vector<TextLineAttr>{});
      auto alignments = j.value("horizontalAlignment", std::vector<ETextHorizontalAlignment>{});

      std::vector<ParagraphAttr> parStyle;
      parStyle.reserve(lineType.size());

      size_t       i = 0;
      auto         defaultAlign = alignments.empty() ? HA_LEFT : alignments.back();
      TextLineAttr defaultLineType;
      while (i < lineType.size() && i < alignments.size())
      {
        parStyle.emplace_back(lineType[i], alignments[i]);
        i++;
      }
      while (i < lineType.size())
      {
        parStyle.emplace_back(lineType[i], defaultAlign);
        i++;
      }
      while (i < alignments.size())
      {
        parStyle.emplace_back(defaultLineType, alignments[i]);
        i++;
      }
      if (m_fontNameVisitor)
      {
        for (const auto& style : textStyle)
        {
          m_fontNameVisitor(style.font.fontName, style.font.subFamilyName);
        }
      }
      p->setParagraph(j.value("content", ""), std::move(textStyle), std::move(parStyle));

      if (bounds.width() == 0 || bounds.height() == 0)
      {
        p->setFrameMode(TL_AUTOWIDTH);
      }
      return p;
    });
}

std::vector<PaintNodePtr> SceneBuilder::fromFrames(const json& j, const glm::mat3& totalMatrix)
{
  std::vector<PaintNodePtr> frames;
  const auto&               fs = getOrDefault(j, "frames");
  for (const auto& e : fs)
  {
    frames.push_back(fromFrame(e, totalMatrix));
  }
  return frames;
}

inline std::vector<PaintNodePtr> SceneBuilder::fromSymbolMasters(
  const json&      j,
  const glm::mat3& totalMatrix)
{
  std::vector<PaintNodePtr> symbols;
  const auto&               symbolMasters = getOrDefault(j, "symbolMaster");
  for (const auto& e : symbolMasters)
  {
    symbols.emplace_back(fromSymbolMaster(e, totalMatrix));
  }
  return symbols;
}

PaintNodePtr SceneBuilder::fromFrame(const json& j, const glm::mat3& totalMatrix)
{
  auto p = makeObjectCommonProperty(
    j,
    totalMatrix,
    [&](std::string name, std::string guid)
    {
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_FRAME, std::move(guid));
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
    {
      p->setContourOption(ContourOption(ECoutourType::MCT_FRAMEONLY, false));
      const auto radius = getStackOptional<std::array<float, 4>>(j, "radius")
                            .value_or(std::array<float, 4>{ 0, 0, 0, 0 });
      p->setFrameRadius(radius);
      const auto& childObjects = getOrDefault(j, "childObjects");
      for (const auto& c : childObjects)
      {
        p->addChild(fromObject(c, matrix));
      }
    });

  return p;
}

PaintNodePtr SceneBuilder::fromSymbolMaster(const json& j, const glm::mat3& totalMatrix)
{
  return makeObjectCommonProperty(
    j,
    totalMatrix,
    [&](std::string name, std::string guid)
    {
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_MASTER, std::move(guid));
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
    {
      const auto radius = getStackOptional<std::array<float, 4>>(j, "radius")
                            .value_or(std::array<float, 4>{ 0, 0, 0, 0 });
      p->setFrameRadius(radius);
      const auto& chidlObject = getOrDefault(j, "childObjects");
      for (const auto& e : chidlObject)
      {
        p->addChild(fromObject(e, matrix));
      }
    });
}

PaintNodePtr SceneBuilder::fromSymbolInstance(const json& j, const glm::mat3& totalMatrix)
{
  return nullptr;
}

std::vector<PaintNodePtr> SceneBuilder::fromTopLevelFrames(
  const json&      j,
  const glm::mat3& totalMatrix)
{
  std::vector<PaintNodePtr> frames;
  for (const auto& e : j)
  {
    frames.push_back(fromFrame(e, totalMatrix));
  }
  return frames;
}

PaintNodePtr SceneBuilder::fromGroup(const json& j, const glm::mat3& totalMatrix)
{
  // auto p = std::make_shared<PaintNode>(j.value("name", ""), VGG_GROUP);
  //  init group properties
  return makeObjectCommonProperty(
    j,
    totalMatrix,
    [&](std::string name, std::string guid)
    {
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_GROUP, std::move(guid));
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
    {
      p->setOverflow(OF_VISIBLE); // Group do not clip inner content
      p->setContourOption(ContourOption(ECoutourType::MCT_UNION, false));
      p->setPaintOption(EPaintStrategy(EPaintStrategy::PS_CHILDONLY));
      const auto& childObjects = getOrDefault(j, "childObjects");
      for (const auto& c : childObjects)
      {
        p->addChild(fromObject(c, matrix));
      }
    });
}

void SceneBuilder::buildImpl(const json& j)
{
  glm::mat3 mat = glm::identity<glm::mat3>();
  mat = glm::scale(mat, glm::vec2(1, -1));
  m_frames = fromTopLevelFrames(getOrDefault(j, "frames"), mat);
}

void SceneBuilder::buildImpl2(const json& j)
{
  glm::mat3 mat = glm::identity<glm::mat3>();
  mat = glm::scale(mat, glm::vec2(1, -1));
  auto                         frames = getOrDefault(j, "frames");
  std::vector<JSONFrameObject> frameObjects;
  m_frames = BuilderImpl::fromTopLevelFrames<JSONFrameObject, JSONModelCastObject>(
    frameObjects,
    mat,
    m_alloc);
}

SceneBuilderResult SceneBuilder::build()
{
  SceneBuilderResult result;
  if (!m_doc)
    return { SceneBuilderResult::EResultType::BUILD_FAILD, std::nullopt };
  const auto& doc = *m_doc;
  if (auto it = doc.find("version");
      it == doc.end() || (it != doc.end() && m_version && *it != *m_version))
    result.type = SceneBuilderResult::EResultType::VERSION_MISMATCH;
  buildImpl(doc);
  if (!m_frames.empty())
  {
    result.root = RootArray();
    for (auto& p : m_frames)
    {
      auto frame = makeFramePtr(std::move(p));
      if (m_resetOrigin)
        frame->resetToOrigin(true);
      result.root->emplace_back(frame);
    }
  }
  m_invalid = true;
  return result;
}

} // namespace VGG::layer
