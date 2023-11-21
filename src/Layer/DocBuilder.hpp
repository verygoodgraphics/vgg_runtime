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
#pragma once
#include "Layer/Core/VType.hpp"
#include "Math/Algebra.hpp"
#include "Math/Math.hpp"
#include "Utility/Log.hpp"
#include "AttrSerde.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Node.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/ImageNode.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/transform2.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <nlohmann/json.hpp>

#include <exception>
#include <memory>
#include <optional>

namespace VGG::layer
{

using namespace nlohmann;

class DocBuilder
{
  std::vector<std::shared_ptr<PaintNode>> m_frames;
  std::vector<std::shared_ptr<PaintNode>> m_symbols;

  inline Bound2 fromBound(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    auto x = j.value("x", 0.f);
    auto y = j.value("y", 0.f);
    auto width = j.value("width", 0.f);
    auto height = j.value("height", 0.f);
    auto topLeft = glm::vec2{ x, y };
    auto bottomRight = glm::vec2{ x + width, y - height };
    convertCoordinateSystem(topLeft, totalMatrix);
    convertCoordinateSystem(bottomRight, totalMatrix);
    return Bound2{ topLeft, bottomRight };
  }

  inline std::tuple<glm::mat3, glm::mat3, glm::mat3> fromMatrix(const nlohmann::json& j)
  {
    std::array<float, 6> v = j;
    auto original = glm::mat3{ glm::vec3{ v[0], v[1], 0 },
                               glm::vec3{ v[2], v[3], 0 },
                               glm::vec3{ v[4], v[5], 1 } };
    const auto [newMatrix, inversed] = convertMatrixCoordinate(original);
    return { original, newMatrix, inversed };
  }

  inline Style fromStyle(const nlohmann::json& j, const Bound2& bound, const glm::mat3& totalMatrix)
  {
    Style style;
    from_json(j, style);
    convertCoordinateSystem(style, bound, totalMatrix);
    return style;
  }

  template<typename F1, typename F2>
  inline std::shared_ptr<PaintNode> makeObjectCommonProperty(const nlohmann::json& j,
                                                             const glm::mat3& totalMatrix,
                                                             F1&& creator,
                                                             F2&& override)
  {
    auto obj = creator(std::move(j.value("name", "")), std::move(j.value("id", "")));
    if (!obj)
      return nullptr;
    auto [originalMatrix, newMatrix, inversedNewMatrix] =
      fromMatrix(j.value("matrix", nlohmann::json::array_t{}));
    const auto convertedMatrix = inversedNewMatrix * totalMatrix * originalMatrix;

    obj->setLocalTransform(newMatrix);
    const auto b = fromBound(j.value("bounds", nlohmann::json::object_t{}), convertedMatrix);
    obj->setBound(b);

    // Pattern point in style are implicitly given by bound, we must supply the points in original
    // coordinates for correct converting
    obj->setStyle(fromStyle(j.value("style", nlohmann::json::object_t{}), b, convertedMatrix));
    obj->style().cornerSmooth = get_opt<float>(j, "cornerSmoothing").value_or(0.f);
    obj->setContextSettings(j.value("contextSettings", ContextSetting()));
    obj->setMaskBy(std::move(j.value("outlineMaskBy", std::vector<std::string>{})));
    obj->setAlphaMaskBy(std::move(j.value("alphaMaskBy", std::vector<AlphaMask>{})));
    const auto maskType = j.value("maskType", EMaskType::MT_None);
    const auto defaultShowType =
      maskType == EMaskType::MT_Outline
        ? MST_Content
        : (maskType == EMaskType::MT_Alpha && false ? MST_Bound : MST_Invisible);
    const auto maskShowType = j.value("maskShowType", defaultShowType);
    obj->setMaskType(maskType);
    obj->setMaskShowType(maskShowType);
    obj->setOverflow(j.value("overflow", EOverflow::OF_Visible));
    obj->setVisible(j.value("visible", true));
    override(obj.get(), convertedMatrix);
    return obj;
  }

  inline std::shared_ptr<PaintNode> makeContour(const nlohmann::json& j,
                                                const nlohmann::json& parent,
                                                const glm::mat3& totalMatrix)
  {
    Contour contour;
    contour.closed = j.value("closed", false);
    const auto& points = get_or_default(j, "points");
    for (const auto& e : points)
    {
      contour.emplace_back(get_opt<glm::vec2>(e, "point").value_or(glm::vec2{ 0, 0 }),
                           get_opt<float>(e, "radius").value_or(0.0),
                           get_opt<glm::vec2>(e, "curveFrom"),
                           get_opt<glm::vec2>(e, "curveTo"),
                           get_opt<int>(e, "cornerStyle"));
    }
    // auto p = std::make_shared<ContourNode>("contour", std::make_shared<Contour>(contour), "");
    auto p = std::make_shared<PaintNode>("contour", VGG_CONTOUR, "");
    p->setOverflow(OF_Visible);
    p->setContourOption(ContourOption{ ECoutourType::MCT_FrameOnly, false });
    convertCoordinateSystem(contour, totalMatrix);
    auto ptr = std::make_shared<Contour>(contour);
    ptr->cornerSmooth = get_opt<float>(parent, "cornerSmoothing").value_or(0.f);
    p->setContourData(std::move(ptr));
    return p;
  }

  inline std::shared_ptr<PaintNode> fromFrame(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    auto p = makeObjectCommonProperty(
      j,
      totalMatrix,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_FRAME, std::move(guid));
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

  inline std::shared_ptr<PaintNode> fromImage(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    return makeObjectCommonProperty(
      j,
      totalMatrix,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<ImageNode>(j.value("name", ""), std::move(guid));
        return p;
      },
      [&](ImageNode* p, const glm::mat3& matrix)
      {
        p->setImage(j.value("imageFileName", ""));
        p->setReplacesImage(j.value("fillReplacesImage", false));
      });
  }

  inline std::shared_ptr<PaintNode> fromText(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    return makeObjectCommonProperty(
      j,
      totalMatrix,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<layer::TextNode>(std::move(name), std::move(guid));
        return p;
      },
      [&](layer::TextNode* p, const glm::mat3& matrix)
      {
        std::string text = j.value("content", "");
        auto lineType = get_stack_optional<std::vector<TextLineAttr>>(j, "lineType")
                          .value_or(std::vector<TextLineAttr>());

        auto defaultAttr = defaultTextAttr();
        defaultAttr.update(j.value("defaultAttr", nlohmann::json::object()), true);
        auto fontAttr = j.value("attr", std::vector<nlohmann::json>{});
        std::vector<TextStyleAttr> textStyleAttrs;
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
          textStyleAttrs.push_back(json);
        }
        p->setParagraph(std::move(text), textStyleAttrs, lineType);
        p->setVerticalAlignment(j.value("verticalAlignment", ETextVerticalAlignment::VA_Top));
        const auto& b = p->getBound();
        p->setFrameMode(j.value("frameMode", ETextLayoutMode::TL_Fixed));
        if (b.width() == 0 || b.height() == 0)
        {
          // for Ai speicific
          p->setFrameMode(ETextLayoutMode::TL_WidthAuto);
        }
        else
        {
          // TODO:: force to Fixed now because the bound computed by text layout is not accurate.
          p->setFrameMode(ETextLayoutMode::TL_Fixed);
        }
        return p;
      });
  }

  inline std::shared_ptr<PaintNode> fromPath(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    return makeObjectCommonProperty(
      j,
      totalMatrix,
      [&, this](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_PATH, std::move(guid));
        return p;
      },
      [&, this](PaintNode* p, const glm::mat3& matrix)
      {
        // const auto& shape = get_or_default(j, "shape");
        const auto shape = j.value("shape", nlohmann::json{});
        p->setChildWindingType(shape.value("windingRule", EWindingType::WR_EvenOdd));
        p->setContourOption(ContourOption(ECoutourType::MCT_ByObjectOps, false));
        p->setPaintOption(PaintOption(EPaintStrategy::PS_SelfOnly));
        const auto shapes = shape.value("subshapes", std::vector<nlohmann::json>{});
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

  std::shared_ptr<PaintNode> fromObject(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    std::shared_ptr<PaintNode> ro;
    auto klass = j.value("class", "");
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

  inline std::shared_ptr<PaintNode> fromGroup(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    // auto p = std::make_shared<PaintNode>(j.value("name", ""), VGG_GROUP);
    //  init group properties
    return makeObjectCommonProperty(
      j,
      totalMatrix,
      [&, this](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_GROUP, std::move(guid));
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

  inline std::vector<std::shared_ptr<PaintNode>> fromFrames(const nlohmann::json& j,
                                                            const glm::mat3& totalMatrix)
  {
    std::vector<std::shared_ptr<PaintNode>> frames;
    const auto& fs = get_or_default(j, "frames");
    for (const auto& e : fs)
    {
      frames.push_back(fromFrame(e, totalMatrix));
    }
    return frames;
  }

  inline std::shared_ptr<PaintNode> fromSymbolInstance(const nlohmann::json& j,
                                                       const glm::mat3& totalMatrix)
  {
    return nullptr;
  }

  std::shared_ptr<PaintNode> fromSymbolMaster(const nlohmann::json& j, const glm::mat3& totalMatrix)
  {
    return makeObjectCommonProperty(
      j,
      totalMatrix,
      [&, this](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_MASTER, std::move(guid));
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

  inline std::vector<std::shared_ptr<PaintNode>> fromSymbolMasters(const nlohmann::json& j,
                                                                   const glm::mat3& totalMatrix)
  {
    std::vector<std::shared_ptr<PaintNode>> symbols;
    const auto& symbolMasters = get_or_default(j, "symbolMaster");
    for (const auto& e : symbolMasters)
    {
      symbols.emplace_back(fromSymbolMaster(e, totalMatrix));
    }
    return symbols;
  }

  void appendSymbolMaster(std::shared_ptr<PaintNode> master)
  {
    m_symbols.push_back(std::move(master));
  }

  inline std::vector<std::shared_ptr<PaintNode>> fromTopLevelFrames(const nlohmann::json& j,
                                                                    const glm::mat3& totalMatrix)
  {
    std::vector<std::shared_ptr<PaintNode>> frames;
    for (const auto& e : j)
    {
      frames.push_back(fromFrame(e, totalMatrix));
    }
    return frames;
  }

  DocBuilder() = default;
  void buildImpl(const nlohmann::json& j, bool resetOrigin)
  {
    glm::mat3 mat = glm::identity<glm::mat3>();
    if (!FLIP_COORD)
      mat = glm::scale(mat, glm::vec2(1, -1));
    m_frames = fromTopLevelFrames(get_or_default(j, "frames"), mat);
    if (false)
    {
      for (const auto& p : m_frames)
      {
        auto t = p->localTransform();
        glm::vec2 scale;
        float angle;
        glm::quat quat;
        glm::vec2 skew;
        glm::vec2 offset;
        glm::vec3 persp;
        decompose(t, scale, angle, quat, skew, offset, persp);
        const auto b = p->getBound();
        auto newMatrix = glm::identity<glm::mat3>();
        newMatrix = glm::translate(newMatrix, -b.topLeft() - offset);
        newMatrix = glm::rotate(newMatrix, (float)math::number::Pi - angle);
        newMatrix = glm::shearX(newMatrix, skew.x);
        newMatrix = glm::shearY(newMatrix, skew.y);
        newMatrix = glm::scale(newMatrix, scale);
        p->setLocalTransform(newMatrix);
        p->setOverflow(EOverflow::OF_Visible);
      }
    }
  }

  static glm::mat3 convertPatternMatrix(const glm::mat3& mat)
  {
    glm::mat3 scale = glm::identity<glm::mat3>();
    scale = glm::scale(scale, glm::vec2(1, -1));
    return scale * mat;
  }

  static std::pair<glm::mat3, glm::mat3> convertMatrixCoordinate(const glm::mat3& mat)
  {
    glm::mat3 scale = glm::identity<glm::mat3>();
    scale = glm::scale(scale, { 1, -1 });
    return { scale * mat * scale, scale * glm::inverse(mat) * scale };
    const auto& v0 = mat[0];
    const auto& v1 = mat[1];
    const auto& v2 = mat[2];
    const auto a = v0[0];
    const auto b = v0[1];
    const auto c = v1[0];
    const auto d = v1[1];
    const auto tx = v2[0];
    const auto ty = v2[1];
    glm::mat3 flipped =
      glm::mat3{ glm::vec3{ a, -b, 0 }, glm::vec3{ c, -d, 0 }, glm::vec3{ tx, -ty, 1 } };
    return { flipped, glm::inverse(flipped) };

    const auto domonator = (a * d - b * c);
    const auto inv = 1.f / domonator;
    glm::mat3 inversed =
      glm::mat3{ glm::vec3{ d * inv, b * inv, 0 },
                 glm::vec3{ -c * inv, -a * inv, 0 },
                 glm::vec3{ -(d * tx + c * ty) * inv, -(b * tx + a * ty) * inv, 1.0 } };
    // return { mat, glm::inverse(mat) };
    return { flipped, glm::inverse(flipped) };
    // return { flipped, inversed };
  }

  static void convertCoordinateSystem(glm::vec2& point, const glm::mat3& totalMatrix)
  {
    // point = totalMatrix * glm::vec3(point, 1.f);
    point.y = -point.y;
  }

  static void convertCoordinateSystem(Contour& contour, const glm::mat3& totalMatrix)
  {
    if (FLIP_COORD)
      return;
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

  static void convertCoordinateSystem(Pattern& pattern,
                                      const Bound2& bound,
                                      const glm::mat3& totalMatrix)
  {
    auto newMatrix = convertMatrixCoordinate(pattern.transform).first;
    glm::vec2 scale;
    float rotate;
    glm::quat quat;
    glm::vec2 skew;
    glm::vec2 offset;
    glm::vec3 persp;
    decompose(newMatrix, scale, rotate, quat, skew, offset, persp);
    pattern.rotate = rotate;
    pattern.scale = scale;
    pattern.offset = offset;
    pattern.transform = newMatrix;
  }

  static void convertCoordinateSystem(Gradient& gradient, const glm::mat3& totalMatrix)
  {
    // convertCoordinateSystem(gradient.from, totalMatrix);
    // convertCoordinateSystem(gradient.to, totalMatrix);
  }

  static void convertCoordinateSystem(Style& style,
                                      const Bound2& bound,
                                      const glm::mat3& totalMatrix)
  {
    for (auto& b : style.borders)
    {
      if (b.pattern)
      {
        convertCoordinateSystem(b.pattern.value(), bound, totalMatrix);
      }
      if (b.gradient)
      {
        convertCoordinateSystem(b.gradient.value(), totalMatrix);
      }
    }
    for (auto& f : style.fills)
    {
      if (f.gradient)
      {
        convertCoordinateSystem(f.gradient.value(), totalMatrix);
      }
      if (f.pattern)
      {
        convertCoordinateSystem(f.pattern.value(), bound, totalMatrix);
      }
    }
    for (auto& s : style.shadows)
    {
      s.offset_y = -s.offset_y;
    }
    for (auto& b : style.blurs)
    {
      convertCoordinateSystem(b.center, totalMatrix);
    }
  }

  static nlohmann::json defaultTextAttr()
  {
    auto j = R"({
        "length":0,
        "fillUseType":0,
        "horizontalAlignment":0,
        "name":"Fira Sans",
        "subFamilyName":"",
        "size":14,
        "kerning":true,
        "letterSpacingValue":0,
        "letterSpacingUnit":0,
        "lineSpaceValue":0,
        "lineSpaceUnit":0,
        "underline":0,
        "linethrough":false,
        "bold":false,
        "italic":false,
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

public:
  static NodeContainer build(const nlohmann::json& j, bool resetOrigin = true)
  {
    DocBuilder builder;
    builder.buildImpl(j, resetOrigin);
    return NodeContainer{ std::move(builder.m_frames), std::move(builder.m_symbols) };
  }
};

} // namespace VGG::layer
