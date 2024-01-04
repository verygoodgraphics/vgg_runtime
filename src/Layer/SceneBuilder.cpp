/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Layer/SceneBuilder.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/ParagraphLayout.hpp"
#include "Layer/ParagraphParser.hpp"
#include "Math/Algebra.hpp"
#include "Math/Math.hpp"
#include "Utility/Log.hpp"
#include "AttrSerde.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/ImageNode.hpp"

namespace
{
class CoordinateConvert
{
public:
  static void convertCoordinateSystem(glm::vec2& point, const glm::mat3& totalMatrix)
  {
    // evaluated form of 'point = totalMatrix * glm::vec3(point, 1.f);'
    point.y = -point.y;
  }

  static void convertCoordinateSystem(Contour& contour, const glm::mat3& totalMatrix)
  {
    for (auto& p : contour)
    {
      convertCoordinateSystem(p.point, totalMatrix);
      if (p.from)
      {
        convertCoordinateSystem(p.from.value(), totalMatrix);
      }
      if (p.to)
      {
        convertCoordinateSystem(p.to.value(), totalMatrix);
      }
    }
  }

  static std::pair<glm::mat3, glm::mat3> convertMatrixCoordinate(const glm::mat3& mat)
  {
    glm::mat3 scale = glm::identity<glm::mat3>();
    scale = glm::scale(scale, { 1, -1 });
    return { scale * mat * scale, scale * glm::inverse(mat) * scale };
  }

  static void convertCoordinateSystem(Pattern& pattern, const glm::mat3& totalMatrix)
  {

    std::visit(
      Overloaded{ [](PatternFill& p) { p.rotation = -p.rotation; },
                  [](PatternFit& p) { p.rotation = -p.rotation; },
                  [](PatternStretch& p)
                  {
                    auto newMatrix = convertMatrixCoordinate(p.transform.matrix()).first;
                    p.transform.setMatrix(newMatrix);
                  },
                  [](PatternTile& p) { p.rotation = -p.rotation; } },
      pattern.instance);
  }

  static void convertCoordinateSystem(Gradient& gradient, const glm::mat3& totalMatrix)
  {
    std::visit(
      Overloaded{
        [&](GradientLinear& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
        },
        [&](GradientRadial& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
          if (auto d = std::get_if<glm::vec2>(&p.ellipse); d)
          {
            convertCoordinateSystem(*d, totalMatrix);
          }
        },
        [&](GradientAngular& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
          if (auto d = std::get_if<glm::vec2>(&p.ellipse); d)
          {
            convertCoordinateSystem(*d, totalMatrix);
          }
        },
        [&](GradientDiamond& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
          if (auto d = std::get_if<glm::vec2>(&p.ellipse); d)
          {
            convertCoordinateSystem(*d, totalMatrix);
          }
        },
        [&](GradientBasic& p) {},
      },
      gradient.instance);
  }

  static void convertCoordinateSystem(TextStyleAttr& textStyle, const glm::mat3& totalMatrix)
  {
    for (auto& f : textStyle.fills)
    {
      std::visit(
        Overloaded{
          [&](Gradient& g) { convertCoordinateSystem(g, totalMatrix); },
          [&](Pattern& p) { convertCoordinateSystem(p, totalMatrix); },
          [&](Color& c) {},
        },
        f.type);
    }
  }

  static void convertCoordinateSystem(Style& style, const glm::mat3& totalMatrix)
  {
    for (auto& b : style.borders)
    {
      std::visit(
        Overloaded{
          [&](Gradient& g) { convertCoordinateSystem(g, totalMatrix); },
          [&](Pattern& p) { convertCoordinateSystem(p, totalMatrix); },
          [&](Color& c) {},
        },
        b.type);
    }
    for (auto& f : style.fills)
    {
      std::visit(
        Overloaded{
          [&](Gradient& g) { convertCoordinateSystem(g, totalMatrix); },
          [&](Pattern& p) { convertCoordinateSystem(p, totalMatrix); },
          [&](Color& c) {},
        },
        f.type);
    }
    for (auto& s : style.shadows)
    {
      s.offsetY = -s.offsetY;
    }
    for (auto& b : style.blurs)
    {
      convertCoordinateSystem(b.center, totalMatrix);
    }
  }
};
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

Bound SceneBuilder::fromBound(const json& j, const glm::mat3& totalMatrix)
{
  auto x = j.value("x", 0.f);
  auto y = j.value("y", 0.f);
  auto width = j.value("width", 0.f);
  auto height = j.value("height", 0.f);
  auto topLeft = glm::vec2{ x, y };
  auto bottomRight = glm::vec2{ x + width, y - height };
  CoordinateConvert::convertCoordinateSystem(topLeft, totalMatrix);
  CoordinateConvert::convertCoordinateSystem(bottomRight, totalMatrix);
  return Bound{ topLeft, bottomRight };
}

Style SceneBuilder::fromStyle(const json& j, const Bound& bound, const glm::mat3& totalMatrix)
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
#ifdef USE_SHARED_PTR
      auto p = makeImageNodePtr(j.value("name", ""), std::move(guid));
#else
      auto p = makeImageNodePtr(m_alloc, j.value("name", ""), std::move(guid));
#endif
      return p;
    },
    [&](ImageNode* p, const glm::mat3& matrix)
    {
      p->setImage(j.value("imageFileName", ""));
      p->setReplacesImage(j.value("fillReplacesImage", false));
    });
}

PaintNodePtr SceneBuilder::fromPath(const json& j, const glm::mat3& totalMatrix)
{
  return makeObjectCommonProperty(
    j,
    totalMatrix,
    [&](std::string name, std::string guid)
    {
#ifdef USE_SHARED_PTR
      auto p = makePaintNodePtr(std::move(name), VGG_PATH, std::move(guid));
#else
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_PATH, std::move(guid));
#endif
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix)
    {
      // const auto& shape = get_or_default(j, "shape");
      const auto shape = j.value("shape", json{});
      p->setChildWindingType(shape.value("windingRule", EWindingType::WR_EvenOdd));
      p->setContourOption(ContourOption(ECoutourType::MCT_ByObjectOps, false));
      p->setPaintOption(PaintOption(EPaintStrategy::PS_SelfOnly));
      const auto shapes = shape.value("subshapes", std::vector<json>{});
      // const auto& shapes = get_or_default(shape, "subshapes");
      for (const auto& subshape : shapes)
      {
        const auto blop = subshape.value("booleanOperation", EBoolOp::BO_None);
        // const auto& geo = get_or_default(subshape, "subGeometry");
        const auto geo = subshape.value("subGeometry", nlohmann::json{});
        const auto klass = geo.value("class", "");
        if (klass == "contour")
        {
          p->addSubShape(makeContour(geo, j, matrix), blop);
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
#ifdef USE_SHARED_PTR
      auto p = makeTextNodePtr(std::move(name), std::move(guid));
#else
      auto p = makeTextNodePtr(m_alloc, std::move(name), std::move(guid));
#endif
      return p;
    },
    [&](layer::TextNode* p, const glm::mat3& matrix)
    {
      p->setVerticalAlignment(j.value("verticalAlignment", ETextVerticalAlignment::VA_Top));
      p->setFrameMode(j.value("frameMode", ETextLayoutMode::TL_Fixed));
      auto anchor = get_stack_optional<std::array<float, 2>>(j, "anchorPoint");
      if (anchor)
      {
        p->setTextAnchor({ (*anchor)[0], (*anchor)[1] });
      }

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

      const auto& b = p->frameBound();
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
      auto         defaultAlign = alignments.empty() ? HA_Left : alignments.back();
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
      p->setParagraph(j.value("content", ""), std::move(textStyle), std::move(parStyle));
      if (b.width() == 0 || b.height() == 0)
      {
        // for Ai speicific
        p->setFrameMode(TL_WidthAuto);
      }
      return p;
    });
}

std::vector<PaintNodePtr> SceneBuilder::fromFrames(const json& j, const glm::mat3& totalMatrix)
{
  std::vector<PaintNodePtr> frames;
  const auto&               fs = get_or_default(j, "frames");
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
  const auto&               symbolMasters = get_or_default(j, "symbolMaster");
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
#ifdef USE_SHARED_PTR
      auto p = makePaintNodePtr(std::move(name), VGG_FRAME, std::move(guid));
#else
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_FRAME, std::move(guid));
#endif
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix)
    {
      p->setContourOption(ContourOption(ECoutourType::MCT_FrameOnly, false));
      const auto radius = get_stack_optional<std::array<float, 4>>(j, "radius");
      p->style().frameRadius = radius;
      const auto& childObjects = get_or_default(j, "childObjects");
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
#ifdef USE_SHARED_PTR
      auto p = makePaintNodePtr(std::move(name), VGG_MASTER, std::move(guid));
#else
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_MASTER, std::move(guid));
#endif
      // appendSymbolMaster(p);
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix)
    {
      const auto radius = get_stack_optional<std::array<float, 4>>(j, "radius");
      p->style().frameRadius = radius;
      const auto& chidlObject = get_or_default(j, "childObjects");
      for (const auto& e : chidlObject)
      {
        p->addChild(fromObject(e, matrix));
      }
    });
}

PaintNodePtr SceneBuilder::makeContour(
  const json&      j,
  const json&      parent,
  const glm::mat3& totalMatrix)
{
  Contour contour;
  contour.closed = j.value("closed", false);
  const auto& points = get_or_default(j, "points");
  for (const auto& e : points)
  {
    contour.emplace_back(
      get_opt<glm::vec2>(e, "point").value_or(glm::vec2{ 0, 0 }),
      get_opt<float>(e, "radius").value_or(0.0),
      get_opt<glm::vec2>(e, "curveFrom"),
      get_opt<glm::vec2>(e, "curveTo"),
      get_opt<int>(e, "cornerStyle"));
  }
  // auto p = std::make_shared<ContourNode>("contour", std::make_shared<Contour>(contour), "");
#ifdef USE_SHARED_PTR
  auto p = makePaintNodePtr("contour", VGG_CONTOUR, "");
#else
  auto p = makePaintNodePtr(m_alloc, "contour", VGG_CONTOUR, "");
#endif
  p->setOverflow(OF_Visible);
  p->setContourOption(ContourOption{ ECoutourType::MCT_FrameOnly, false });
  CoordinateConvert::convertCoordinateSystem(contour, totalMatrix);
  auto ptr = std::make_shared<Contour>(contour);
  ptr->cornerSmooth = get_opt<float>(parent, "cornerSmoothing").value_or(0.f);
  p->setContourData(std::move(ptr));
  return p;
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
#ifdef USE_SHARED_PTR
      auto p = makePaintNodePtr(std::move(name), VGG_GROUP, std::move(guid));
#else
      auto p = makePaintNodePtr(m_alloc, std::move(name), VGG_GROUP, std::move(guid));
#endif
      return p;
    },
    [&, this](PaintNode* p, const glm::mat3& matrix)
    {
      p->setOverflow(OF_Visible); // Group do not clip inner content
      p->setContourOption(ContourOption(ECoutourType::MCT_Union, false));
      p->setPaintOption(EPaintStrategy(EPaintStrategy::PS_ChildOnly));
      const auto& childObjects = get_or_default(j, "childObjects");
      for (const auto& c : childObjects)
      {
        p->addChild(fromObject(c, matrix));
      }
    });
}

void SceneBuilder::buildImpl(const json& j, bool resetOrigin)
{
  glm::mat3 mat = glm::identity<glm::mat3>();
  mat = glm::scale(mat, glm::vec2(1, -1));
  m_frames = fromTopLevelFrames(get_or_default(j, "frames"), mat);
  if (resetOrigin)
  {
    for (const auto& p : m_frames)
    {
      // const auto m = p->transform();
      const auto b = p->frameBound();
      p->transform().setTranslate(-b.topLeft().x, -b.topLeft().y);
      p->setOverflow(EOverflow::OF_Visible);
    }
  }
}

} // namespace VGG::layer
