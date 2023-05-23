#pragma once
#include "Basic/Mask.h"
#include "PaintNode.h"
#include "VGGType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"

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
  EBoolOp blop;
};
/*
 * the children of the path node as subshapes of shape
 * */
class PathNode final : public PaintNode
{
public:
  struct Subshape
  {
    std::vector<Contour> contours;
  };

  struct Shape
  {
    EWindingType windingRule;
    Subshape subshape;
  } shape;

public:
  PathNode(const std::string& name);
  void paintEvent(SkCanvas* canvas) override;
  Mask asOutlineMask(const glm::mat3* mat) override;
  void addSubShape(std::shared_ptr<PaintNode> node, EBoolOp op);
  void addSubShape(const Contour& ctr, EBoolOp op);

protected:
  void drawContour(SkCanvas* canvas, const SkPath* outlineMask);
  SkPath makePath();
  sk_sp<SkRuntimeEffect> maskEffect;
};
} // namespace VGG
