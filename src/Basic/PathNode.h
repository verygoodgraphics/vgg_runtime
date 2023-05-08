#pragma once
#include "PaintNode.h"
#include "VGGType.h"
#include "include/core/SkCanvas.h"

#include <charconv>
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
  float radius = 0.0;
  std::optional<glm::vec2> from;
  std::optional<glm::vec2> to;
  std::optional<int> cornerStyle;

  PointAttr(glm::vec2 point,
            float radius,
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

  PointMode mode() const
  {
    if (from.has_value() || to.has_value())
      return PointMode::DISCONNECTED;
    return PointMode::STRAIGHT;
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
  PathNode(const std::string& name);
  void Paint(SkCanvas* canvas) override;

protected:
  void drawContour(SkCanvas* canvas);
};
} // namespace VGG
