#include "ImageNode.h"
#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "Scene.hpp"
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

    auto mask = makeMaskBy(BO_Intersection);
    if (mask.outlineMask.isEmpty() == false)
    {
      SkPaint maskPaint;
      maskPaint.setStyle(SkPaint::Style::kFill_Style);
      maskPaint.setColor(SkColors::kBlue);
      mask.outlineMask.transform(SkMatrix::Scale(1, -1));
      // canvas->drawPath(mask.outlineMask, maskPaint);
      // canvas->clipPath(mask.outlineMask);
    }
    SkSamplingOptions opt;
    SkPaint p;
    p.setColor(SkColors::kRed);
    p.setStyle(SkPaint::kFill_Style);

    SkPath testPath;
    std::vector<SkPoint> points;
    points.resize(mask.outlineMask.countPoints());
    mask.outlineMask.getPoints(points.data(), points.size());
    // testPath.addPoly({ { 0, 0 }, { 100, 0 }, { 100, 100 }, { 0, 100 } }, true);
    testPath.addPoly(points.data(), points.size(), true);
    SkPaint testPaint;
    testPaint.setStyle(SkPaint::Style::kFill_Style);
    testPaint.setColor(SkColors::kRed);
    canvas->drawPath(testPath, testPaint);
    canvas->clipPath(testPath);

    SkPaint imagePaint;
    imagePaint.setAlphaf(0.5);
    canvas->drawImageRect(image, toSkRect(this->bound), opt, &imagePaint);
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
