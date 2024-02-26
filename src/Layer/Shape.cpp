#include "Layer/Core/Shape.hpp"
#include <pathops/SkPathOps.h>
#include "Layer/VSkia.hpp"

namespace VGG::layer
{
void Shape::op(const Shape& shape, EBoolOp op)
{
  SkPath p = asPath();
  auto   sop = toSkPathOp(op);
  Op(p, shape.asPath(), sop, &p);
  setPath(p);
}
} // namespace VGG::layer
