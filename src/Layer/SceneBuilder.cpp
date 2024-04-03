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
#include <variant>

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
