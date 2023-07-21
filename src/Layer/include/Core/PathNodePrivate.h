#pragma once

#include "Core/Node.hpp"
#include "Core/VType.h"
#include "Core/PathNode.h"
#include "core/SkShader.h"

namespace VGG
{
class PathNode__pImpl
{
  VGG_DECL_API(PathNode);

public:
  EWindingType windingRule;
  sk_sp<SkShader> testShader;
  PathNode__pImpl(PathNode* api)
    : q_ptr(api)
  {
  }

  SkPath makePath(const std::vector<std::pair<SkPath, EBoolOp>>& ct);

  void drawContour(SkCanvas* canvas,
                   const ContextSetting& settings,
                   const Style& style,
                   EWindingType windingRule,
                   const std::vector<std::pair<SkPath, EBoolOp>>& ct,
                   const Bound2& bound,
                   bool hasFill);

  void drawShadow(SkCanvas* canvas,
                  const SkPath& skPath,
                  const Shadow& s,
                  SkPaint::Style style,
                  const Bound2& bound);

  void drawInnerShadow(SkCanvas* canvas,
                       const SkPath& skPath,
                       const Shadow& s,
                       SkPaint::Style style,
                       const Bound2& bound);

  sk_sp<SkShader> getGradientShader(const Gradient& g, const Bound2& bound);

  void drawPathBorder(SkCanvas* canvas,
                      const SkPath& skPath,
                      const Border& b,
                      float globalAlpha,
                      const Bound2& bound);
};
} // namespace VGG
