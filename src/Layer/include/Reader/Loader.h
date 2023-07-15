#pragma once

#include "Core/Attrs.h"
#include "Core/ContourNode.h"
#include "Core/VGGType.h"
#include "Core/Node.hpp"
#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "nlohmann/json.hpp"
#include "Core/SymbolNode.h"
#include "Core/PaintNode.h"
#include "Core/PathNode.h"
#include "Core/TextNode.h"
#include "Core/ImageNode.h"
#include "Core/GroupNode.h"

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
    auto p0 = it.value()[0].get<float>();
    auto p1 = it.value()[1].get<float>();
    return glm::vec2(p0, p1);
  }
  return std::nullopt;
}

template<>
inline std::optional<glm::vec2> get_opt(const nlohmann::json& obj, const char* key)
{
  if (auto it = obj.find(key); it != obj.end())
  {
    auto p0 = it.value()[0].get<float>();
    auto p1 = it.value()[1].get<float>();
    return glm::vec2(p0, p1);
  }
  return std::nullopt;
}

class NlohmannBuilder
{
public:
  static inline glm::mat3 fromMatrix(const nlohmann::json& j)
  {
    auto v = j.at("matrix").get<std::vector<double>>();
    assert(v.size() == 6);
    const auto m = glm::mat3{ glm::vec3{ v[0], v[1], 0 },
                              glm::vec3{ v[2], v[3], 0 },
                              glm::vec3{ v[4], v[5], 1 } };
    return m;
  }
  static inline Bound2 fromBound(const nlohmann::json& j)
  {
    auto x = j["x"].get<float>(); // convert to skia coordinate system
    auto y = j["y"].get<float>();
    auto width = j["width"];
    auto height = j["height"];
    return Bound2{ x, y, width, height };
  }

  static inline std::tuple<Bound2, glm::mat3> fromTransform(const nlohmann::json& j)
  {
    return { fromBound(j["bounds"]), fromMatrix(j) };
  }

  static inline void fromObjectCommonProperty(const nlohmann::json& j, PaintNode* obj)
  {
    // all properties that render object cares about
    std::tie(obj->m_bound, obj->m_transform) = fromTransform(j);
    obj->style = j.at("style").get<Style>();
    obj->m_contextSetting = j.at("contextSettings").get<ContextSetting>();
    obj->maskedBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
    obj->maskType = (EMaskType)j.at("maskType").get<int>();
    obj->m_overflow = (EOverflow)j.at("overflow").get<int>();
    obj->guid = j.at("id").get<std::string>();
    obj->setVisible(j.at("visible").get<bool>());
  }

  static inline std::shared_ptr<ContourNode> fromContour(const nlohmann::json& j)
  {
    Contour contour;
    contour.closed = j["closed"];
    for (const auto& e : j["points"])
    {
      const auto p = e["point"];
      contour.emplace_back(glm::vec2{ p[0], p[1] },
                           get_opt<float>(e, "radius").value_or(0.0),
                           get_opt<glm::vec2>(e, "curveFrom"),
                           get_opt<glm::vec2>(e, "curveTo"),
                           get_opt<int>(e, "cornerStyle"));
    }
    return std::make_shared<ContourNode>("contour", std::make_shared<Contour>(contour));
  }

  static inline std::shared_ptr<PaintNode> fromFrame(const nlohmann::json& j)
  {
    auto p = std::make_shared<PaintNode>(j["name"], ObjectType::VGG_FRAME);
    return p;
  }

  static inline std::shared_ptr<ImageNode> fromImage(const nlohmann::json& j)
  {
    auto p = std::make_shared<ImageNode>(j["name"]);
    fromObjectCommonProperty(j, p.get());
    p->setImage(j["imageFileName"]);
    if (auto it = j.find("fillReplacesImage"); it != j.end())
    {
      p->setReplacesImage(it.value());
    }
    return p;
  }

  static inline std::shared_ptr<SymbolInstanceNode> fromSymbolInstance(const nlohmann::json& j)
  {
    auto ins = std::make_shared<SymbolInstanceNode>(j["name"]);
    ins->symbolID = j["symbolID"];
    fromObjectCommonProperty(j, ins.get());
    return ins;
  }

  static inline std::shared_ptr<TextNode> fromText(const nlohmann::json& j)
  {
    auto p = std::make_shared<TextNode>("Text");
    std::string text = j.at("content");
    auto lineType = get_stack_optional<std::vector<TextLineAttr>>(j, "lineType")
                      .value_or(std::vector<TextLineAttr>());
    p->setParagraph(std::move(text), j.at("attr"), lineType);
    p->setVerticalAlignment(j.at("verticalAlignment"));
    p->setFrameMode(j.at("frameMode"));
    return p;
  }

  static inline std::shared_ptr<PathNode> fromPath(const nlohmann::json& j)
  {
    auto p = std::make_shared<PathNode>(j["name"]);
    fromObjectCommonProperty(j, p.get());
    const auto shape = j["shape"];
    p->setWindingRule(shape["windingRule"]);
    for (const auto& subshape : shape["subshapes"])
    {
      const auto blop = subshape["booleanOperation"];
      const auto geo = subshape["subGeometry"];
      const auto klass = geo["class"];
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
        // p->addSubShape(fromGroup(geo), blop);
        p->addSubShape(fromFrame(geo), blop);
      }
    }
    return p;
  }

  static inline std::shared_ptr<PaintNode> fromObject(const nlohmann::json& j)
  {
    std::shared_ptr<PaintNode> ro;
    auto klass = j["class"];
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
    else
    {
      // error
      return nullptr;
    }
    fromObjectCommonProperty(j, ro.get());
    return ro;
  }

  static inline std::shared_ptr<GroupNode> fromGroup(const nlohmann::json& j)
  {
    auto p = std::make_shared<GroupNode>(j["name"]);
    // init group properties
    fromObjectCommonProperty(j, p.get());
    for (const auto& c : j["childObjects"])
    {
      p->addChild(fromObject(c));
    }
    return p;
  }

  static inline std::shared_ptr<PaintNode> fromLayer(const nlohmann::json& j)
  {
    auto p = std::make_shared<PaintNode>("Layer", VGG_LAYER);
    p->m_transform = glm::mat3(1);
    fromObjectCommonProperty(j, p.get());
    for (const auto& e : j["childObjects"])
    {
      p->addChild(fromObject(e));
    }
    return p;
  }

  static inline std::vector<std::shared_ptr<PaintNode>> fromLayers(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> layers;
    for (const auto& e : j["layers"])
    {
      layers.emplace_back(fromLayer(e));
    }
    return layers;
  }

  static inline std::vector<std::shared_ptr<PaintNode>> fromArtboard(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> artboards;
    for (const auto& e : j["artboard"])
    {
      auto p = std::make_shared<PaintNode>(e["name"], VGG_ARTBOARD);
      const auto bg =
        get_stack_optional<VGGColor>(e, "backgroundColor").value_or(VGGColor{ 1, 1, 1, 1 });
      p->setBackgroundColor(bg);
      fromObjectCommonProperty(e, p.get());

      const auto origin = p->localTransform();
      const auto b = p->getBound();
      auto t = glm::translate(origin, glm::vec2{ -origin[2][0], -origin[2][1] });
      t = glm::translate(t, glm::vec2{ -b.topLeft.x, -b.topLeft.y });
      p->setLocalTransform(t);
      auto layers = fromLayers(e);
      for (const auto& l : layers)
      {
        p->addChild(l);
      }
      artboards.push_back(p);
    }
    return artboards;
  }

  static inline std::shared_ptr<SymbolMasterNode> fromSymbolMaster(const nlohmann::json& j)
  {
    auto p = std::make_shared<SymbolMasterNode>(j["name"]);
    fromObjectCommonProperty(j, p.get());
    p->symbolID = j["symbolID"];
    for (const auto& e : j["childObjects"])
    {
      p->addChild(fromObject(e));
    }
    return p;
  }

  static inline std::vector<std::shared_ptr<SymbolMasterNode>> fromSymbolMasters(
    const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<SymbolMasterNode>> symbols;
    if (j.contains("symbolMaster"))
    {
      for (const auto& e : j["symbolMaster"])
      {
        symbols.emplace_back(fromSymbolMaster(e));
      }
    }
    return symbols;
  }
};

} // namespace VGG
