#pragma once
#include "Basic/Mask.h"
#include "PaintNode.h"
#include "ContourNode.h"
#include "VGGType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"

#include <optional>
namespace VGG
{

// TODO:: PathNode is not a good name.
// It likes a special container that provides bool operation for PaintNode object
class PathNode final : public PaintNode
{
  // struct Subshape
  // {
  //   std::vector<Contour> contours;
  // };
  //
  // struct Shape
  // {
  //   EWindingType windingRule;
  //   Subshape subshape;
  // } shape;

  EWindingType windingRule;

public:
  PathNode(const std::string& name);
  void paintEvent(SkCanvas* canvas) override;
  void setWindingRule(EWindingType type)
  {
    windingRule = type;
  }
  Mask asOutlineMask(const glm::mat3* mat) override;
  void addSubShape(std::shared_ptr<PaintNode> node, EBoolOp op);

  void renderOrderPass(SkCanvas* canvas) override
  {
    // do not render child object for path node
  }

protected:
  void drawContour(SkCanvas* canvas, const SkPath* outlineMask);
  SkPath makePath();
};
} // namespace VGG
