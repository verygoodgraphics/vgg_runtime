#pragma once

#include "Node.hpp"
#include "nlohmann/json.hpp"
#include "RenderTreeDef.hpp"
#include <memory>

namespace VGG
{
using namespace nlohmann;

inline std::shared_ptr<GroupNode> fromGroup(const nlohmann::json& j);

inline glm::mat3 fromMatrix(const nlohmann::json& j)
{
  auto v = j.at("matrix").get<std::vector<double>>();
  assert(v.size() == 6);
  return glm::mat3{ glm::vec3{ v[0], v[2], v[4] },
                    glm::vec3{ v[1], v[3], v[5] },
                    glm::vec3{ 0, 0, 1 } };
}

inline Bound2 fromBound(const nlohmann::json& j)
{
  auto x = j["x"];
  auto y = j["y"];
  auto width = j["width"];
  auto height = j["height"];
  return Bound2{ x, y, width, height };
}

inline std::tuple<Bound2, glm::mat3> fromTransform(const nlohmann::json& j)
{
  return { fromBound(j["bounds"]), fromMatrix(j) };
}

inline void fromObjectCommonProperty(const nlohmann::json& j, PaintNode* obj)
{
  // all properties that render object cares about
  std::tie(obj->bound, obj->transform) = fromTransform(j);
}

inline Contour fromContour(const nlohmann::json& j)
{
  Contour contour;
  contour.closed = j["closed"];
  for (const auto& e : j["points"])
  {
    auto p = e["point"];
    contour.emplace_back(p[0], p[1], e["radius"], e["cornerStyle"]);
  }
  return contour;
}

inline std::shared_ptr<ImageNode> fromImage(const nlohmann::json& j)
{
  return std::make_shared<ImageNode>("Image");
}

inline std::shared_ptr<SymbolInstanceNode> fromSymbolInstance(const nlohmann::json& j)
{
  auto ins = std::make_shared<SymbolInstanceNode>(j["name"]);
  ins->symbolID = j["symbolID"];
  fromObjectCommonProperty(j, ins.get());
  return ins;
}

inline std::shared_ptr<TextNode> fromText(const nlohmann::json& j)
{
  return std::make_shared<TextNode>("Text");
}

inline std::shared_ptr<PathNode> fromPath(const nlohmann::json& j)
{
  auto p = std::make_shared<PathNode>(j["name"]);
  fromObjectCommonProperty(j, p.get());
  for (const auto& subshape : j["shape"]["subshapes"])
  {
    auto geo = subshape["subGeometry"];
    auto klass = geo["class"];
    if (klass == "contour")
    {
      p->contour = fromContour(geo);
    }
    else if (klass == "path")
    {
      p->pushChildBack(fromPath(geo));
    }
    else if (klass == "image")
    {
      p->pushChildBack(fromImage(geo));
    }
    else if (klass == "text")
    {
      p->pushChildBack(fromText(geo));
    }
    else if (klass == "group")
    {
      p->pushChildBack(fromGroup(j));
    }
    else if (klass == "symbolInstance")
    {
      p->pushChildBack(fromSymbolInstance(geo));
    }
  }
  return p;
}

inline std::shared_ptr<PaintNode> fromObject(const nlohmann::json& j)
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
  fromObjectCommonProperty(j, ro.get());
  return ro;
}

inline std::shared_ptr<GroupNode> fromGroup(const nlohmann::json& j)
{
  auto p = std::make_shared<GroupNode>("group");
  // init group properties
  fromObjectCommonProperty(j, p.get());
  for (const auto& c : j["childObjects"])
  {
    p->pushChildBack(fromObject(c));
  }
  return p;
}

inline std::shared_ptr<PaintNode> fromLayer(const nlohmann::json& j)
{
  auto p = std::make_shared<PaintNode>("Layer", VGG_LAYER);
  p->transform = glm::mat3(1);
  for (const auto& e : j["childObjects"])
  {
    p->pushChildBack(fromObject(e));
  }
  return p;
}

inline std::vector<std::shared_ptr<PaintNode>> fromLayers(const nlohmann::json& j)
{
  std::vector<std::shared_ptr<PaintNode>> layers;
  for (const auto& e : j["layers"])
  {
    layers.emplace_back(fromLayer(e));
  }
  return layers;
}

inline std::vector<std::vector<std::shared_ptr<PaintNode>>> fromArtboard(const nlohmann::json& j)
{
  std::vector<std::vector<std::shared_ptr<PaintNode>>> boards;
  for (const auto& e : j["artboard"])
  {
    boards.emplace_back(fromLayers(e));
  }
  return boards;
}

inline std::shared_ptr<SymbolMasterNode> fromSymbolMaster(const nlohmann::json& j)
{
  auto p = std::make_shared<SymbolMasterNode>(j["name"]);
  fromObjectCommonProperty(j, p.get());
  p->symbolID = j["symbolID"];
  for (const auto& e : j["childObjects"])
  {
    p->pushChildBack(fromObject(e));
  }
  return p;
}

inline std::vector<std::shared_ptr<SymbolMasterNode>> fromSymbolMasters(const nlohmann::json& j)
{
  std::vector<std::shared_ptr<SymbolMasterNode>> symbols;
  for (const auto& e : j["symbolMaster"])
  {
    symbols.emplace_back(fromSymbolMaster(e));
  }
  return symbols;
}

} // namespace VGG
