#pragma once
#include "Common/Config.h"
#include "Core/VType.h"
#include "Core/PaintNode.h"
#include "Attrs.h"
#include "SkiaImpl/VSkia.h"

#include <vector>

namespace VGG
{

struct Contour : public std::vector<PointAttr>
{
  bool closed = true;
  EBoolOp blop;
};

using ContourPtr = std::shared_ptr<Contour>;

class VGG_EXPORTS ContourNode final : public PaintNode
{
  ContourPtr data{ nullptr };

public:
  ContourNode(const std::string& name, ContourPtr data, std::string guid)
    : PaintNode(name, VGG_CONTOUR, std::move(guid))
    , data(std::move(data))
  {
    setOverflow(OF_Visible);
  }
  Contour* contour() const
  {
    return data.get();
  }

  Mask asOutlineMask(const glm::mat3* mat)
  {
    Mask mask;
    if (data)
    {
      mask.outlineMask = getSkiaPath(*data, data->closed);
    }
    if (mat)
    {
      mask.outlineMask.transform(toSkMatrix(*mat));
    }
    return mask;
  }
};
}; // namespace VGG
