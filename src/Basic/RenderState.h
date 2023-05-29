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
  std::vector<SkPath> pathStack;
  std::optional<SkPath> currentPath;

public:
  RenderState()
  {
  }

  void pushMask(const SkPath& path)
  {
    pathStack.push_back(std::move(path));
    currentPath = std::nullopt;
  }

  void popMask()
  {
    pathStack.pop_back();
    currentPath = std::nullopt;
  }

  const SkPath& makePath()
  {
    if (currentPath)
    {
      SkPath path;
      for (const auto& p : pathStack)
      {
        path.addPath(p);
      }
      currentPath = path;
    }
    return currentPath.value();
  }

  void clearPath()
  {
    pathStack.clear();
    currentPath = std::nullopt;
  }
};

}; // namespace VGG
