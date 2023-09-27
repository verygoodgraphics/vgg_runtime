#pragma once
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

namespace VGG
{
// NOLINTBEGIN
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

template<typename K>
inline const nlohmann::json& get_or_default(const nlohmann::json& j, K&& key)
{
  if (auto it = j.find(key); it != j.end())
  {
    return it.value();
  }

  static const nlohmann::json s_json;
  return s_json;
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

// NOLINTEND

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
    const auto& points = get_or_default(j, "points");
    for (const auto& e : points)
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
    m_frames = fromTopLevelFrames(get_or_default(j, "frames"));
  }

public:
  static NodeContainer build(const nlohmann::json& j)
  {
    NlohmannBuilder builder;
    builder.buildImpl(j);
    return NodeContainer{ std::move(builder.m_frames), std::move(builder.m_symbols) };
  }
};

} // namespace VGG
