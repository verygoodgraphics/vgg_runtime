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
    // SkSamplingOptions opt;
    // const auto mi = image->imageInfo();
    // auto skrect = toSkRect(this->bound);
    // SkMatrix mat = toSkMatrix(this->transform);
    // this->shader = image->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, opt, mat);
  }
  if (image)
  {
    auto mask = makeMaskBy(BO_Intersection);
    if (mask.outlineMask.isEmpty() == false)
    {
      // SkPaint maskPaint;
      // maskPaint.setStyle(SkPaint::Style::kFill_Style);
      // maskPaint.setColor(SkColors::kBlue);
      // canvas->drawPath(mask.outlineMask, maskPaint);
      canvas->clipPath(mask.outlineMask);
    }
    SkSamplingOptions opt;
    canvas->save();
    canvas->scale(1, -1);
    canvas->drawImageRect(image, toSkRect(this->bound), opt);
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
