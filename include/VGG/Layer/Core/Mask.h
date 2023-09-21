#pragma once
#include "VGG/Layer/Config.h"

#include <include/core/SkPath.h>

namespace VGG
{

// TODO:: More features added later
class VGG_EXPORTS Mask
{
public:
  SkPath outlineMask;
  Mask(){};
  void addMask(const Mask& mask)
  {
    outlineMask.addPath(mask.outlineMask);
  }

  Mask& operator=(Mask&&) noexcept = default;
  Mask(Mask&&) noexcept = default;
  Mask& operator=(const Mask&) = default;
  Mask(const Mask&) = default;
};
}; // namespace VGG
