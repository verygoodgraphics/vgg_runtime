#pragma once

#include "Core/Attrs.h"
#include "Core/ContourNode.h"
#include "Core/VType.h"
#include "Core/Node.h"
#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "nlohmann/json.hpp"
#include "Core/PaintNode.h"
#include "Core/PathNode.h"
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
public:
  static inline glm::mat3 fromMatrix(const nlohmann::json& j)
  {
    const auto v = j.value("matrix", std::array<float, 6>{ 1, 0, 0, 1, 0, 0 });
    const auto m = glm::mat3{ glm::vec3{ v[0], v[1], 0 },
                              glm::vec3{ v[2], v[3], 0 },
                              glm::vec3{ v[4], v[5], 1 } };
    return m;
  }
  static inline Bound2 fromBound(const nlohmann::json& j)
  {
    auto x = j.value("x", 0.f);
    auto y = j.value("y", 0.f);
    auto width = j.value("width", 0.f);
    auto height = j.value("height", 0.f);
    return Bound2{ x, y, width, height };
  }

  static inline std::tuple<Bound2, glm::mat3> fromTransform(const nlohmann::json& j)
  {
    return { fromBound(j.value("bounds", nlohmann::json{})), fromMatrix(j) };
  }

  template<typename F1, typename F2>
  static inline std::shared_ptr<PaintNode> makeObjectCommonProperty(const nlohmann::json& j,
                                                                    F1&& creator,
                                                                    F2&& overridor)
  {
    auto obj = creator(std::move(j.value("name", "")), std::move(j.value("id", "")));
    const auto [bound, transform] = fromTransform(j);
    obj->setBound(bound);
    obj->setLocalTransform(transform);
    obj->setStyle(j.value("style", Style()));
    obj->setContectSettings(j.value("contextSettings", ContextSetting()));
    obj->setMaskBy(std::move(j.value("outlineMaskBy", std::vector<std::string>{})));
    obj->setMaskType(j.value("maskType", EMaskType::MT_None));
    obj->setOverflow(j.value("overflow", EOverflow::OF_Visible));
    obj->setVisible(j.value("visible", true));
    overridor(obj.get());
    return obj;
  }

  static inline std::shared_ptr<ContourNode> fromContour(const nlohmann::json& j)
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
    return std::make_shared<ContourNode>("contour", std::make_shared<Contour>(contour), "");
  }

  static inline std::shared_ptr<PaintNode> fromFrame(const nlohmann::json& j)
  {
    auto p = makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_FRAME, std::move(guid));
        return p;
      },
      [&j](PaintNode* p)
      {
        p->setMaskOption(MaskOption(EMaskCoutourType::MCT_Union, false));
        const auto radius = get_stack_optional<std::array<float, 4>>(j, "radius")
                              .value_or(std::array<float, 4>{ 0.0f, 0.f, 0.f, 0.f });
        p->style().frameRadius = radius;
        for (const auto& c : j.value("childObjects", std::vector<nlohmann::json>{}))
        {
          p->addChild(fromObject(c));
        }
      });

    return p;
  }

  static inline std::shared_ptr<PaintNode> fromImage(const nlohmann::json& j)
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

  static inline std::shared_ptr<PaintNode> fromText(const nlohmann::json& j)
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

  static inline std::shared_ptr<PaintNode> fromPath(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_PATH, std::move(guid));
        return p;
      },
      [&j](PaintNode* p)
      {
        const auto shape = j.value("shape", nlohmann::json{});
        p->setChildWindingType(shape.value("windingRule", EWindingType::WR_EvenOdd));
        p->setMaskOption(MaskOption(EMaskCoutourType::MCT_ByObjectOps, false));
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
        }
      });
  }

  static inline std::shared_ptr<PaintNode> fromObject(const nlohmann::json& j)
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
    else
    {
      // error
      return nullptr;
    }
    return ro;
  }

  static inline std::shared_ptr<PaintNode> fromGroup(const nlohmann::json& j)
  {
    // auto p = std::make_shared<PaintNode>(j.value("name", ""), VGG_GROUP);
    //  init group properties
    return makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_GROUP, std::move(guid));
        return p;
      },
      [&j](PaintNode* p)
      {
        p->setOverflow(OF_Visible); // Group do not clip inner content
        p->setMaskOption(MaskOption(EMaskCoutourType::MCT_Union, false));
        for (const auto& c : j.value("childObjects", std::vector<nlohmann::json>{}))
        {
          p->addChild(fromObject(c));
        }
      });
  }

  static inline std::shared_ptr<PaintNode> fromLayer(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_LAYER, std::move(guid));
        return p;
      },
      [&j](PaintNode* p)
      {
        p->setLocalTransform(glm::mat3(1));
        for (const auto& e : j.value("childObjects", std::vector<nlohmann::json>{}))
        {
          p->addChild(fromObject(e));
        }
      });
  }

  static inline std::vector<std::shared_ptr<PaintNode>> fromLayers(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> layers;
    for (const auto& e : j.value("layers", std::vector<nlohmann::json>{}))
    {
      layers.emplace_back(fromLayer(e));
    }
    return layers;
  }

  static inline std::vector<std::shared_ptr<PaintNode>> fromArtboard(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> artboards;
    for (const auto& e : j.value("artboard", std::vector<nlohmann::json>{}))
    {
      auto artboard = makeObjectCommonProperty(
        e,
        [&e](std::string name, std::string guid)
        {
          auto p = std::make_shared<PaintNode>(e.value("name", ""), VGG_ARTBOARD, std::move(guid));
          return p;
        },
        [&e](PaintNode* p)
        {
          const auto bg =
            get_stack_optional<Color>(e, "backgroundColor").value_or(Color{ 1, 1, 1, 1 });
          p->setBackgroundColor(bg);

          auto t = p->localTransform();
          const auto b = p->getBound();
          t = glm::translate(t, glm::vec2{ -t[2][0], -t[2][1] });
          t = glm::translate(t, glm::vec2{ -b.topLeft.x, -b.topLeft.y });
          p->setLocalTransform(t);
          auto layers = fromLayers(e);
          for (const auto& l : layers)
          {
            p->addChild(l);
          }
        });
      artboards.push_back(artboard);
    }
    return artboards;
  }

  static inline std::shared_ptr<PaintNode> fromSymbolInstance(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_MASTER, std::move(guid));
        return p;
      },
      [&j](PaintNode* p) {});
  }

  static inline std::shared_ptr<PaintNode> fromSymbolMaster(const nlohmann::json& j)
  {
    return makeObjectCommonProperty(
      j,
      [&j](std::string name, std::string guid)
      {
        auto symbolID = j.value("symbolID", "");
        auto p = std::make_shared<PaintNode>(std::move(name), VGG_MASTER, std::move(symbolID));
        return p;
      },
      [&j](PaintNode* p)
      {
        for (const auto& e : j.value("childObjects", std::vector<nlohmann::json>()))
        {
          p->addChild(fromObject(e));
        }
      });
  }

  static inline std::vector<std::shared_ptr<PaintNode>> fromSymbolMasters(const nlohmann::json& j)
  {
    std::vector<std::shared_ptr<PaintNode>> symbols;
    for (const auto& e : j.value("symbolMaster", std::vector<nlohmann::json>{}))
    {
      symbols.emplace_back(fromSymbolMaster(e));
    }
    return symbols;
  }
};

} // namespace VGG
