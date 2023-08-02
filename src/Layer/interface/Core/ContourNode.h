#pragma once
#include "Common/Config.h"
#include "Core/VType.h"
#include "Core/PaintNode.h"
#include "Attrs.h"
#include "SkiaImpl/VSkia.h"

#include <vector>

namespace VGG
{

class VGG_EXPORTS ContourNode final : public PaintNode
{
  ContourPtr m_data{ nullptr };

public:
  ContourNode(const std::string& name, ContourPtr data, std::string guid)
    : PaintNode(name, VGG_CONTOUR, std::move(guid))
    , m_data(std::move(data))
  {
    setOverflow(OF_Visible);
  }
  Contour* pointData() const
  {
    return m_data.get();
  }

  SkPath makeContourImpl(ContourOption option, const glm::mat3* mat)
  {
    SkPath mask;
    if (m_data)
    {
      mask = getSkiaPath(*m_data, m_data->closed);
    }
    if (mat)
    {
      mask.transform(toSkMatrix(*mat));
    }
    return mask;
  }

  // Mask asOutlineMask(const glm::mat3* mat)
  // {
  //   Mask mask;
  //   mask.outlineMask = makeContourImpl(maskOption(), mat);
  //   if (mask.outlineMask.isEmpty())
  //   {
  //     WARN("Contour [%s] is empty", getName().c_str());
  //   }
  //   return mask;
  // }
};
}; // namespace VGG
