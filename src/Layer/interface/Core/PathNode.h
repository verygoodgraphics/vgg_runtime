#pragma once
#include "Common/Config.h"
#include "Core/Mask.h"
#include "Core/Node.h"
#include "Core/PaintNode.h"
#include "Core/ContourNode.h"
#include "Core/VType.h"
#include "core/SkCanvas.h"
#include "core/SkMatrix.h"
#include "core/SkPath.h"
#include "core/SkShader.h"
#include "effects/SkRuntimeEffect.h"

#include <optional>
namespace VGG
{

class PathNode__pImpl;

// TODO:: PathNode is not a good name.
// It likes a special container that provides bool operation for PaintNode object
class VGG_EXPORTS PathNode : public PaintNode
{
  VGG_DECL_IMPL(PathNode)
  // EWindingType windingRule;
  // sk_sp<SkShader> testShader;

public:
  PathNode(const std::string& name, std::string guid);
  void paintEvent(SkCanvas* canvas) override;
  void setWindingRule(EWindingType type);
  Mask asOutlineMask(const glm::mat3* mat) override;
  void addSubShape(std::shared_ptr<PaintNode> node, EBoolOp op);
  void renderOrderPass(SkCanvas* canvas) override
  {
    // do not render child object for path node
  }

  bool hasFill() const;

  virtual ~PathNode();

protected:
  virtual void paintFill(SkCanvas* canvas,
                         float globalAlpha,
                         const Style& style,
                         const SkPath& skPath,
                         const Bound2& bound);
};
} // namespace VGG
