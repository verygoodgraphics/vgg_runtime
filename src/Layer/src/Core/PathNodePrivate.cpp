#include "Core/PathNodePrivate.h"

#include "Core/Geometry.hpp"
#include "Core/VType.h"
#include "Core/PathNode.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "SkiaImpl/VSkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/gpu/GrTypes.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkBlurMask.h"
#include <algorithm>
#include <core/SkCanvas.h>

namespace VGG
{

SkPath PathNode__pImpl::makePath(const std::vector<std::pair<SkPath, EBoolOp>>& ct)
{
  // return skpath;
  assert(ct.size() >= 1);

  if (ct.size() == 1)
  {
    return ct[0].first;
  }

  std::vector<SkPath> res;
  SkPath skPath = ct[0].first;
  for (int i = 1; i < ct.size(); i++)
  {
    SkPath rhs;
    auto op = ct[i].second;
    if (op != BO_None)
    {
      auto skop = toSkPathOp(op);
      rhs = ct[i].first;
      Op(skPath, rhs, skop, &skPath);
    }
    else
    {
      res.push_back(skPath);
      skPath = ct[i].first;
    }
    op = ct[i].second;
  }
  res.push_back(skPath);

  SkPath paths;
  for (const auto s : res)
  {
    paths.addPath(s);
  }
  return paths;
}

} // namespace VGG
