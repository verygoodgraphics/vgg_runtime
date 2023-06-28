#include <Core/ImageNode.h>
#include <Core/PathNode.h>
#include "Core/VGGType.h"
#include "Core/VGGUtils.h"
#include "SkiaBackend/SkiaImpl.h"
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

ImageNode::ImageNode(const std::string& name)
  : PathNode(name)
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

void ImageNode::paintFill(SkCanvas* canvas, float globalAlpha, const SkPath& skPath)
{
  if (!image)
  {
    image = loadImage(guid, Scene::getResRepo());
  }
  if (image)
  {
    auto mask = makeMaskBy(BO_Intersection);
    if (mask.outlineMask.isEmpty() == false)
    {
      canvas->save();
      canvas->clipPath(mask.outlineMask);
    }
    // SkPaint p;
    // p.setColor(SK_ColorGREEN);
    // p.setStrokeWidth(2);
    // canvas->drawRect(toSkRect(getBound()), p);

    SkSamplingOptions opt;
    canvas->drawImageRect(image, toSkRect(getBound()), opt);

    if (mask.outlineMask.isEmpty() == false)
    {
      canvas->restore();
    }
  }
}

} // namespace VGG
