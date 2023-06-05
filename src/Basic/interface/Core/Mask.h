#pragma once

#include "include/core/SkPath.h"

namespace VGG
{

// TODO:: More features added later
class Mask
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
