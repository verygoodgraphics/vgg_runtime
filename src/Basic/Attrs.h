#pragma once
#include "VGGType.h"
#include <optional>
#include <vector>
#include <stdint.h>

namespace VGG
{
struct ContextSetting
{
  EBlendMode BlendMode;
  float Opacity[2];
  bool IsolateBlending;
  int TransparencyKnockoutGroup;
};
} // namespace VGG
