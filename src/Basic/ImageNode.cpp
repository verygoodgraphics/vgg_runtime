#include "ImageNode.h"
#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "Basic/SkiaBackend/SkiaImpl.h"
#include "Basic/Scene.hpp"
#include "glm/matrix.hpp"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTileMode.h"

#include <include/core/SkCanvas.h>
#include <include/core/SkImage.h>

namespace VGG
{

ImageNode::ImageNode(const std::string& name)
  : PaintNode(name, VGG_IMAGE)
{
}

void ImageNode::paintEvent(SkCanvas* canvas)
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
      canvas->clipPath(mask.outlineMask);
    }
    SkSamplingOptions opt;
    canvas->save();
    canvas->scale(1, -1);
    canvas->drawImageRect(image, toSkRect(this->m_bound), opt);
    canvas->restore();
  }
}

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

} // namespace VGG
