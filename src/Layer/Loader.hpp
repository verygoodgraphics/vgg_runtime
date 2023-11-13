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
#include "Utility/Log.hpp"
#include "AttrSerde.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Node.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/ImageNode.hpp"

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

class NlohmannBuilder
{
  std::vector<std::shared_ptr<PaintNode>> m_frames;
  std::vector<std::shared_ptr<PaintNode>> m_symbols;

  inline glm::mat3 fromMatrix(const nlohmann::json& j)
  {
    auto v = j.value("matrix", std::array<float, 6>{ 1, 0, 0, 1, 0, 0 });
    return flipCoord(glm::mat3{ glm::vec3{ v[0], v[1], 0 },
                                glm::vec3{ v[2], v[3], 0 },
                                glm::vec3{ v[4], v[5], 1 } });
  }
  inline Bound2 fromBound(const nlohmann::json& j)
  {
    auto x = j.value("x", 0.f);
    auto y = j.value("y", 0.f);
    const auto topLeft = flipCoord(glm::vec2{ x, y });
    auto width = j.value("width", 0.f);
    auto height = j.value("height", 0.f);
    return Bound2{ topLeft, width, height };
  }

  inline std::tuple<Bound2, glm::mat3> fromTransform(const nlohmann::json& j)
  {
    return { fromBound(get_or_default(j, "bounds")), fromMatrix(j) };
  }

  template<typename F1, typename F2>
  inline std::shared_ptr<PaintNode> makeObjectCommonProperty(const nlohmann::json& j,
                                                             F1&& creator,
                                                             F2&& override)
  {
    auto obj = creator(std::move(j.value("name", "")), std::move(j.value("id", "")));
    if (!obj)
      return nullptr;
    const auto [bound, transform] = fromTransform(j);
    obj->setBound(bound);
    obj->setLocalTransform(transform);
    auto style = j.value("style", Style());
    obj->setStyle(style);
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
    override(obj.get());
    return obj;
  }

  inline std::shared_ptr<PaintNode> fromContour(const nlohmann::json& j,
                                                const nlohmann::json& parent)
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
    auto ptr = std::make_shared<Contour>(contour);
    ptr->cornerSmooth = get_opt<float>(parent, "cornerSmoothing").value_or(0.f);
    p->setContourData(std::move(ptr));
    return p;
  }

  inline std::shared_ptr<PaintNode> fromFrame(const nlohmann::json& j)
  {
    auto p = makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_FRAME, std::move(guid));
        return p;
      },
      [this, &j](PaintNode* p)
      {
        p->setContourOption(ContourOption(ECoutourType::MCT_FrameOnly, false));
        const auto radius = get_stack_optional<std::array<float, 4>>(j, "radius");
        p->style().frameRadius = radius;
        const auto& childObjects = get_or_default(j, "childObjects");
        for (const auto& c : childObjects)
        {
          p->addChild(fromObject(c));
        }
      });

    return p;
  }

  inline std::shared_ptr<PaintNode> fromImage(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<ImageNode>(j.value("name", ""), std::move(guid));
        return p;
      },
      [&j](ImageNode* p)
      {
        p->setImage(j.value("imageFileName", ""));
        p->setReplacesImage(j.value("fillReplacesImage", false));
      });
  }

  inline std::shared_ptr<PaintNode> fromText(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<layer::TextNode>(std::move(name), std::move(guid));
        return p;
      },
      [&j](layer::TextNode* p)
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

  inline std::shared_ptr<PaintNode> fromPath(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [this, &j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_PATH, std::move(guid));
        return p;
      },
      [this, &j](PaintNode* p)
      {
        const auto& shape = get_or_default(j, "shape");
        p->setChildWindingType(shape.value("windingRule", EWindingType::WR_EvenOdd));
        p->setContourOption(ContourOption(ECoutourType::MCT_ByObjectOps, false));
        p->setPaintOption(PaintOption(EPaintStrategy::PS_SelfOnly));
        const auto& shapes = get_or_default(shape, "subshapes");
        for (const auto& subshape : shapes)
        {
          const auto blop = subshape.value("booleanOperation", EBoolOp::BO_None);
          const auto& geo = get_or_default(subshape, "subGeometry");
          const auto klass = geo.value("class", "");
          if (klass == "contour")
          {
            p->addSubShape(fromContour(geo, j), blop);
          }
          else if (klass == "path")
          {
            p->addSubShape(fromPath(geo), blop);
          }
          else if (klass == "image")
          {
            p->addSubShape(fromImage(geo), blop);
          }
          else if (klass == "text")
          {
            p->addSubShape(fromText(geo), blop);
          }
          else if (klass == "group")
          {
            p->addSubShape(fromGroup(geo), blop);
          }
          else if (klass == "symbolInstance")
          {
            p->addSubShape(fromSymbolInstance(geo), blop);
          }
          else if (klass == "frame")
          {
            p->addSubShape(fromFrame(geo), blop);
          }
          else if (klass == "symbolMaster")
          {
            p->addSubShape(fromSymbolMaster(geo), blop);
          }
        }
      });
  }

  std::shared_ptr<PaintNode> fromObject(const nlohmann::json& j)
  {
    std::shared_ptr<PaintNode> ro;
    auto klass = j.value("class", "");
    if (klass == "group")
    {
      ro = fromGroup(j);
    }
    else if (klass == "path")
    {
      ro = fromPath(j);
    }
    else if (klass == "image")
    {
      ro = fromImage(j);
    }
    else if (klass == "text")
    {
      ro = fromText(j);
    }
    else if (klass == "symbolInstance")
    {
      ro = fromSymbolInstance(j);
    }
    else if (klass == "frame")
    {
      ro = fromFrame(j);
    }
    else if (klass == "symbolMaster")
    {
      ro = fromSymbolMaster(j);
    }
    else
    {
      // error
      return nullptr;
    }
    return ro;
  }

  inline std::shared_ptr<PaintNode> fromGroup(const nlohmann::json& j)
  {
    // auto p = std::make_shared<PaintNode>(j.value("name", ""), VGG_GROUP);
    //  init group properties
    return makeObjectCommonProperty(
      j,
      [this, &j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_GROUP, std::move(guid));
        return p;
      },
      [this, &j](PaintNode* p)
      {
        p->setOverflow(OF_Visible); // Group do not clip inner content
        p->setContourOption(ContourOption(ECoutourType::MCT_Union, false));
        p->setPaintOption(EPaintStrategy(EPaintStrategy::PS_ChildOnly));
        const auto& childObjects = get_or_default(j, "childObjects");
        for (const auto& c : childObjects)
        {
          p->addChild(fromObject(c));
        }
      });
  }

  inline std::vector<std::shared_ptr<PaintNode>> fromFrames(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> frames;
    const auto& fs = get_or_default(j, "frames");
    for (const auto& e : fs)
    {
      frames.push_back(fromFrame(e));
    }
    return frames;
  }

  inline std::shared_ptr<PaintNode> fromSymbolInstance(const nlohmann::json& j)
  {
    return nullptr;
  }

  std::shared_ptr<PaintNode> fromSymbolMaster(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [this, &j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_MASTER, std::move(guid));
        // appendSymbolMaster(p);
        return p;
      },
      [this, &j](PaintNode* p)
      {
        const auto radius = get_stack_optional<std::array<float, 4>>(j, "radius");
        p->style().frameRadius = radius;
        const auto& chidlObject = get_or_default(j, "childObjects");
        for (const auto& e : chidlObject)
        {
          p->addChild(fromObject(e));
        }
      });
  }

  inline std::vector<std::shared_ptr<PaintNode>> fromSymbolMasters(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> symbols;
    const auto& symbolMasters = get_or_default(j, "symbolMaster");
    for (const auto& e : symbolMasters)
    {
      symbols.emplace_back(fromSymbolMaster(e));
    }
    return symbols;
  }

  void appendSymbolMaster(std::shared_ptr<PaintNode> master)
  {
    m_symbols.push_back(std::move(master));
  }

  inline std::vector<std::shared_ptr<PaintNode>> fromTopLevelFrames(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> frames;
    for (const auto& e : j)
    {
      frames.push_back(fromFrame(e));
    }
    for (const auto& p : frames)
    {
      // auto t = p->localTransform();
      // const auto b = p->getBound();
      // p->setLocalTransform(t);
      // p->setOverflow(EOverflow::OF_Visible);
    }
    return frames;
  }

  NlohmannBuilder() = default;
  void buildImpl(const nlohmann::json& j)
  {
    m_frames = fromTopLevelFrames(get_or_default(j, "frames"));
  }

public:
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
  static NodeContainer build(const nlohmann::json& j)
  {
    NlohmannBuilder builder;
    builder.buildImpl(j);
    return NodeContainer{ std::move(builder.m_frames), std::move(builder.m_symbols) };
  }
};

} // namespace VGG::layer
