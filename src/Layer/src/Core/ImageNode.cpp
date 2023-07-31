#include <Core/ImageNode.h>
#include <Core/PathNode.h>
#include <core/SkBlendMode.h>
#include <core/SkRect.h>
#include "Core/VType.h"
#include "SkiaImpl/VSkia.h"
#include "glm/matrix.hpp"
#include "core/SkColor.h"
#include "core/SkMatrix.h"
#include "core/SkPaint.h"
#include "core/SkPathTypes.h"
#include "core/SkPoint.h"
#include "core/SkTileMode.h"
#include "core/SkCanvas.h"
#include "core/SkImage.h"

namespace VGG
{

ImageNode::ImageNode(const std::string& name, std::string guid)
  : PathNode(name, std::move(guid))
{
}

Mask ImageNode::asOutlineMask(const glm::mat3* mat)
{
  Mask mask;
  auto rect = toSkRect(getBound());
  mask.outlineMask.addRect(rect);
  if (mat)
  {
    mask.outlineMask.transform(toSkMatrix(*mat));
  }
  return mask;
}

// void ImageNode::paintEvent(SkCanvas* canvas)
// {
// }

void ImageNode::setImage(const std::string& guid)
{
  this->guid = guid;
}

const std::string& ImageNode::getImageGUID() const
{
  return guid;
}

void ImageNode::setReplacesImage(bool fill)
{
  this->fillReplacesImage = fill;
}

bool ImageNode::fill() const
{
  return this->fillReplacesImage;
}

void ImageNode::paintFill(SkCanvas* canvas,
                          float globalAlpha,
                          const Style& style,
                          const SkPath& skPath,
                          const Bound2& bound)
{
  if (!image)
  {
    image = loadImage(guid, Scene::getResRepo());
  }
  if (image)
  {
    if (!shader)
    {
      const auto& b = bound;
      shader =
        getImageShader(image, b.width(), b.height(), EImageFillType::IFT_Stretch, 1.0, false);
    }
    auto mask = makeMaskBy(BO_Intersection);
    if (mask.outlineMask.isEmpty() == false)
    {
      canvas->save();
      canvas->clipPath(mask.outlineMask);
    }

    SkPaint p;
    p.setShader(shader);
    canvas->drawPaint(p);
    // Another weird drawing method
    // SkSamplingOptions opt;
    // const auto& b = getBound();
    // SkRect imageRect = SkRect::MakeXYWH(b.topLeft.x, -b.topLeft.y, b.width(), b.height());
    // canvas->save();
    // canvas->scale(1, -1);
    // canvas->drawImageRect(image, imageRect, opt);
    // canvas->restore();

    if (mask.outlineMask.isEmpty() == false)
    {
      canvas->restore();
    }
  }
}

} // namespace VGG
