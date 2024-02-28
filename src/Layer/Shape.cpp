#include "Layer/Core/Shape.hpp"
#include <pathops/SkPathOps.h>
#include "Layer/VSkia.hpp"

namespace VGG::layer
{
void ShapePath::op(const ShapePath& shape, EBoolOp op)
{
  SkPath p = asPath();
  auto   sop = toSkPathOp(op);
  Op(p, shape.asPath(), sop, &p);
  setPath(p);
}

ShapePath::~ShapePath()
{
}
} // namespace VGG::layer
