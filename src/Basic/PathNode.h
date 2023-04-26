#pragma once
#include "PaintNode.h"
#include "VGGType.h"

#include <optional>
namespace VGG
{

enum PointMode
{
  STRAIGHT = 1,
  MIRRORED = 2,
  ASYMMETRIC = 3,
  DISCONNECTED = 4,
};
struct PointAttr
{
  glm::vec2 point;
  std::optional<float> radius;
  std::optional<glm::vec2> from;
  std::optional<glm::vec2> to;
  std::optional<int> cornerStyle;
  PointMode mode;

  PointAttr(glm::vec2 point,
            std::optional<float> radius,
            std::optional<glm::vec2> from,
            std::optional<glm::vec2> to,
            std::optional<int> cornerStyle)
    : point(point)
    , radius(radius)
    , from(from)
    , to(to)
    , cornerStyle(cornerStyle)
  {
  }
};

struct Contour : public std::vector<PointAttr>
{
  bool closed = true;
};
/*
 * the children of the path node as subshapes of shape
 * */
class PathNode final : public PaintNode
{
public:
  struct Subshape
  {
    BoolOp blop;
    std::optional<Contour> contour;
  };
  struct Shape
  {
    WindingType windingRule;
    Subshape subshape;
  } shape;

public:
  PathNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_PATH)
  {
  }
  void Paint(SkCanvas* canvas) override{

  }
};
} // namespace VGG
