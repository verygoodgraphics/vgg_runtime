#pragma once
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

#include <optional>
#include <stack>

namespace VGG
{
class RenderState
{
  float alpha{ 1.0 };

public:
  RenderState()
  {
  }
};

}; // namespace VGG
