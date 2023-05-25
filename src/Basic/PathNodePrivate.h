#pragma once

#include "Basic/VGGType.h"
#include "PathNode.h"

SkPath makePath(const std::vector<std::pair<SkPath, EBoolOp>>& ct);

void drawContour(SkCanvas* canvas,
                 const SkPath* outlineMask,
                 const ContextSetting& settings,
                 const Style& style,
                 EWindingType windingRule,
                 const std::vector<std::pair<SkPath, EBoolOp>>& ct,
                 const Bound2& bound);

void drawShadow(SkCanvas* canvas,
                const SkPath& skPath,
                const Shadow& s,
                const SkPath* outlineMask,
                bool inner);

sk_sp<SkShader> getGradientShader(const VGGGradient& g, const glm::vec2& size);
