#pragma once

#include "Common/Config.h"
#include "core/SkPath.h"

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
};
}; // namespace VGG
