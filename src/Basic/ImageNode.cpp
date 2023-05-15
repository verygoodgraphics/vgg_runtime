#include "ImageNode.h"
#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "Scene.hpp"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTileMode.h"

#include <include/core/SkCanvas.h>
#include <include/core/SkImage.h>

namespace VGG
{

std::unordered_map<std::string, sk_sp<SkImage>> ImageNode::SkiaImageRepo = {};

ImageNode::ImageNode(const std::string& name)
  : PaintNode(name, VGG_IMAGE)
{
}

void ImageNode::Paint(SkCanvas* canvas)
{
  if (!image)
  {
    reloadImage();
  }
  if (image)
  {

    // draw mask
    auto mask = makeMaskBy(BO_Intersection);
    if (mask.outlineMask.isEmpty() == false)
    {
      SkPaint maskPaint;
      maskPaint.setStyle(SkPaint::Style::kFill_Style);
      maskPaint.setColor(SkColors::kBlue);
      mask.outlineMask.transform(SkMatrix::Scale(1, -1));
      canvas->drawPath(mask.outlineMask, maskPaint);
      canvas->clipPath(mask.outlineMask);
    }

    // apply scale
    SkSamplingOptions opt;
    canvas->drawImageRect(image, toSkRect(this->bound), opt);
    // canvas->restore();
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

void ImageNode::reloadImage()
{
  if (guid.empty())
    return;
  if (!image)
  {
    if (auto it = SkiaImageRepo.find(guid); it != SkiaImageRepo.end())
    {
      image = it->second;
    }
    else
    {
      auto repo = Scene::getResRepo();
      if (auto it = repo.find(guid); it != repo.end())
      {
        auto data = SkData::MakeWithCopy(it->second.data(), it->second.size());
        if (!data)
        {
          WARN("Make SkData failed");
          return;
        }
        sk_sp<SkImage> skImage = SkImage::MakeFromEncoded(data);
        if (!skImage)
        {
          WARN("Make SkImage failed");
          return;
        }
        SkiaImageRepo[guid] = skImage;
        this->image = skImage;
        SkSamplingOptions opt;

        const auto mi = image->imageInfo();
        auto skrect = toSkRect(this->bound);
        SkMatrix mat = toSkMatrix(this->transform);
        // mat.postScale(skrect.width() / mi.width(), skrect.height() / mi.height());
        this->shader = skImage->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, opt, mat);
      }
      else
      {
        WARN("Cannot find %s from resources repository", this->guid.c_str());
      }
    }
  }
}

} // namespace VGG
