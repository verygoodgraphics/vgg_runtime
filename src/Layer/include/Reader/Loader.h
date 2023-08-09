#pragma once

#include "Core/Attrs.h"
#include "Core/VType.h"
#include "Core/Node.h"
#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "nlohmann/json.hpp"
#include "Core/PaintNode.h"
#include "Core/TextNode.h"
#include "Core/ImageNode.h"

#include "Reader/AttrSerde.h"
#include <glm/gtc/matrix_transform.hpp>
#include <exception>
#include <memory>
#include <optional>

namespace VGG
{
using namespace nlohmann;
template<typename T>
inline std::optional<T> get_opt(const nlohmann::json& obj, const std::string& key)
{
  if (auto p = obj.find(key); p != obj.end())
    return p.value().get<T>();
  return std::nullopt;
}

template<typename T>
inline std::optional<T> get_opt(const nlohmann::json& obj, const char* key)
{
  if (auto p = obj.find(key); p != obj.end())
    return p.value().get<T>();
  return std::nullopt;
}

template<>
inline std::optional<glm::vec2> get_opt(const nlohmann::json& obj, const std::string& key)
{
  if (auto it = obj.find(key); it != obj.end())
  {
    auto v = obj.value(key, std::array<float, 2>{ 0, 0 });
    auto p0 = v[0];
    auto p1 = v[1];
    return glm::vec2(p0, p1);
  }
  return std::nullopt;
}

template<>
inline std::optional<glm::vec2> get_opt(const nlohmann::json& obj, const char* key)
{
  if (auto it = obj.find(key); it != obj.end())
  {
    auto v = obj.value(key, std::array<float, 2>{ 0, 0 });
    auto p0 = v[0];
    auto p1 = v[1];
    return glm::vec2(p0, p1);
  }
  return std::nullopt;
}

class NlohmannBuilder
{
  std::vector<std::shared_ptr<PaintNode>> m_frames;
  std::vector<std::shared_ptr<PaintNode>> m_symbols;

  inline glm::mat3 fromMatrix(const nlohmann::json& j)
  {
    const auto v = j.value("matrix", std::array<float, 6>{ 1, 0, 0, 1, 0, 0 });
    const auto m = glm::mat3{ glm::vec3{ v[0], v[1], 0 },
                              glm::vec3{ v[2], v[3], 0 },
                              glm::vec3{ v[4], v[5], 1 } };
    return m;
  }
  inline Bound2 fromBound(const nlohmann::json& j)
  {
    auto x = j.value("x", 0.f);
    auto y = j.value("y", 0.f);
    auto width = j.value("width", 0.f);
    auto height = j.value("height", 0.f);
    return Bound2{ x, y, width, height };
  }

  inline std::tuple<Bound2, glm::mat3> fromTransform(const nlohmann::json& j)
  {
    return { fromBound(j.value("bounds", nlohmann::json{})), fromMatrix(j) };
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
    obj->setStyle(j.value("style", Style()));
    obj->setContectSettings(j.value("contextSettings", ContextSetting()));
    obj->setMaskBy(std::move(j.value("outlineMaskBy", std::vector<std::string>{})));
    obj->setMaskType(j.value("maskType", EMaskType::MT_None));
    obj->setOverflow(j.value("overflow", EOverflow::OF_Visible));
    obj->setVisible(j.value("visible", true));
    override(obj.get());
    return obj;
  }

  inline std::shared_ptr<PaintNode> fromContour(const nlohmann::json& j)
  {
    Contour contour;
    contour.closed = j.value("closed", false);
    for (const auto& e : j.value("points", nlohmann::json{}))
    {
      const auto p = e.value("point", std::array<float, 2>{ 0, 0 });
      contour.emplace_back(glm::vec2{ p[0], p[1] },
                           get_opt<float>(e, "radius").value_or(0.0),
                           get_opt<glm::vec2>(e, "curveFrom"),
                           get_opt<glm::vec2>(e, "curveTo"),
                           get_opt<int>(e, "cornerStyle"));
    }
    // auto p = std::make_shared<ContourNode>("contour", std::make_shared<Contour>(contour), "");
    auto p = std::make_shared<PaintNode>("contour", VGG_CONTOUR, "");
    p->setOverflow(OF_Visible);
    p->setContourOption(ContourOption{ ECoutourType::MCT_FrameOnly, false });
    p->setContourData(std::make_shared<Contour>(contour));
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
        for (const auto& c : j.value("childObjects", std::vector<nlohmann::json>{}))
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
        auto p = std::make_shared<TextNode>(std::move(name), std::move(guid));
        return p;
      },
      [&j](TextNode* p)
      {
        std::string text = j.value("content", "");
        auto lineType = get_stack_optional<std::vector<TextLineAttr>>(j, "lineType")
                          .value_or(std::vector<TextLineAttr>());
        p->setParagraph(std::move(text), j.value("attr", std::vector<TextAttr>{}), lineType);
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
        const auto shape = j.value("shape", nlohmann::json{});
        p->setChildWindingType(shape.value("windingRule", EWindingType::WR_EvenOdd));
        p->setContourOption(ContourOption(ECoutourType::MCT_ByObjectOps, false));
        p->setPaintOption(PaintOption(EPaintStrategy::PS_SelfOnly));
        for (const auto& subshape : shape.value("subshapes", std::vector<nlohmann::json>{}))
        {
          const auto blop = subshape.value("booleanOperation", EBoolOp::BO_None);
          const auto geo = subshape.value("subGeometry", nlohmann::json{});
          const auto klass = geo.value("class", "");
          if (klass == "contour")
          {
            p->addSubShape(fromContour(geo), blop);
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
        for (const auto& c : j.value("childObjects", std::vector<nlohmann::json>{}))
        {
          p->addChild(fromObject(c));
        }
      });
  }

  inline std::vector<std::shared_ptr<PaintNode>> fromFrames(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> frames;
    for (const auto& e : j.value("frames", std::vector<nlohmann::json>{}))
    {
      frames.push_back(fromFrame(e));
    }
    return frames;
  }

  inline std::shared_ptr<PaintNode> fromSymbolInstance(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [this, &j](std::string name, std::string guid) -> std::shared_ptr<PaintNode>
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_INSTANCE, std::move(guid));
        auto& instances = Scene::instanceObjects();
        if (auto it = instances.find(p->guid()); it == instances.end())
        {
          std::string masterID = j.value("masterId", "");
          if (!masterID.empty())
          {
            instances[p->guid()] =
              std::move(std::pair<std::weak_ptr<PaintNode>, std::string>{ p, masterID });
          }
          else
          {
            WARN("master id is empty referenced by [%s].", p->guid().c_str());
          }
        }
        else
        {
          WARN("Instance [%s] is duplicated.", p->guid().c_str());
        }
        return p;
      },
      [this, &j](PaintNode* p) {});
  }

  std::shared_ptr<PaintNode> fromSymbolMaster(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [this, &j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_MASTER, std::move(guid));
        appendSymbolMaster(p);
        return p;
      },
      [this, &j](PaintNode* p)
      {
        for (const auto& e : j.value("childObjects", std::vector<nlohmann::json>()))
        {
          p->addChild(fromObject(e));
        }
      });
  }

  inline std::vector<std::shared_ptr<PaintNode>> fromSymbolMasters(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> symbols;
    for (const auto& e : j.value("symbolMaster", std::vector<nlohmann::json>{}))
    {
      symbols.emplace_back(fromSymbolMaster(e));
    }
    return symbols;
  }

  void appendSymbolMaster(std::shared_ptr<PaintNode> master)
  {
    m_symbols.push_back(std::move(master));
  }

  inline std::vector<std::shared_ptr<PaintNode>> fromTopLevelFrames(
    const std::vector<nlohmann::json>& j)
  {
    std::vector<std::shared_ptr<PaintNode>> frames;
    for (const auto& e : j)
    {
      frames.push_back(fromFrame(e));
    }
    for (const auto& p : frames)
    {
      auto t = p->localTransform();
      const auto b = p->getBound();
      t = glm::translate(t, glm::vec2{ -t[2][0], -t[2][1] });
      t = glm::translate(t, glm::vec2{ -b.topLeft.x, -b.topLeft.y });
      p->setLocalTransform(t);
    }
    return frames;
  }

  NlohmannBuilder() = default;
  void buildImpl(const nlohmann::json& j)
  {
    m_frames = fromTopLevelFrames(j.value("frames", std::vector<nlohmann::json>{}));
  }

public:
  static FormatRepresentation build(const nlohmann::json& j)
  {
    NlohmannBuilder builder;
    builder.buildImpl(j);
    return FormatRepresentation{ std::move(builder.m_frames), std::move(builder.m_symbols) };
  }
};

} // namespace VGG
