#pragma once
#include "PaintNode.h"
#include "VGGType.h"

#include <optional>
namespace VGG
{

struct PointAttr
{
  glm::vec2 point;
  std::optional<float> radius;
  std::optional<glm::vec2> curveFrom;
  std::optional<glm::vec2> curveTo;
  std::optional<int> cornerStyle;
  PointAttr(glm::vec2 point,
            std::optional<float> radius,
            std::optional<glm::vec2> curveFrom,
            std::optional<glm::vec2> curveTo,
            std::optional<int> cornerStyle)
    : point(point)
    , radius(radius)
    , curveFrom(curveFrom)
    , curveTo(curveTo)
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

  void Paint(SkCanvas* canvas) override
  {
    if (shape.subshape.contour)
    {
      drawContour(shape.subshape.contour.value());
    }
  }

protected:
  void drawContour(const Contour& contour)
  {
  }
};
} // namespace VGG
