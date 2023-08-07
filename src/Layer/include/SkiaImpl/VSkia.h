#pragma once
#include "Core/VType.h"
#include "Core/Attrs.h"
#include "Scene/Scene.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkImage.h"
#include "include/effects/SkImageFilters.h"
#include "include/pathops/SkPathOps.h"

#include <core/SkFontStyle.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <vector>
#include <unordered_map>
#include <string>
using namespace VGG;

extern std::unordered_map<std::string, sk_sp<SkImage>> g_skiaImageRepo;

#define SWITCH_MAP_ITEM_BEGIN(var)                                                                 \
  switch (var)                                                                                     \
  {

#define SWITCH_MAP_ITEM_DEF(from, to)                                                              \
  case from:                                                                                       \
    return to;

#define SWITCH_MAP_ITEM_DEF_NULL(from) case from:;

#define SWITCH_MAP_ITEM_END(fallback)                                                              \
  default:                                                                                         \
    return fallback;                                                                               \
    }

inline SkPaint::Join toSkPaintJoin(VGG::ELineJoin join)
{
  SWITCH_MAP_ITEM_BEGIN(join)
  SWITCH_MAP_ITEM_DEF(LJ_Miter, SkPaint::kMiter_Join)
  SWITCH_MAP_ITEM_DEF(LJ_Round, SkPaint::kRound_Join)
  SWITCH_MAP_ITEM_DEF(LJ_Bevel, SkPaint::kBevel_Join)
  SWITCH_MAP_ITEM_END(SkPaint::kMiter_Join)
}

inline SkBlendMode toSkBlendMode(EBlendMode mode)
{
  SWITCH_MAP_ITEM_BEGIN(mode)
  SWITCH_MAP_ITEM_DEF(BM_Normal, SkBlendMode::kSrcOver)
  SWITCH_MAP_ITEM_DEF(BM_Darken, SkBlendMode::kDarken)
  SWITCH_MAP_ITEM_DEF(BM_Multiply, SkBlendMode::kMultiply)
  SWITCH_MAP_ITEM_DEF(BM_Color_burn, SkBlendMode::kColorBurn)
  SWITCH_MAP_ITEM_DEF(BM_Lighten, SkBlendMode::kLighten)
  SWITCH_MAP_ITEM_DEF(BM_Screen, SkBlendMode::kScreen)
  SWITCH_MAP_ITEM_DEF(BM_Color_dodge, SkBlendMode::kColorDodge)
  SWITCH_MAP_ITEM_DEF(BM_Overlay, SkBlendMode::kOverlay)
  SWITCH_MAP_ITEM_DEF(BM_Soft_light, SkBlendMode::kSoftLight)
  SWITCH_MAP_ITEM_DEF(BM_Hard_light, SkBlendMode::kHardLight)
  SWITCH_MAP_ITEM_DEF(BM_Difference, SkBlendMode::kDifference)
  SWITCH_MAP_ITEM_DEF(BM_Exclusion, SkBlendMode::kExclusion)
  SWITCH_MAP_ITEM_DEF(BM_Hue, SkBlendMode::kHue)
  SWITCH_MAP_ITEM_DEF(BM_Saturation, SkBlendMode::kSaturation)
  SWITCH_MAP_ITEM_DEF(BM_Color, SkBlendMode::kColor)
  SWITCH_MAP_ITEM_DEF(BM_Luminosity, SkBlendMode::kColor)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Plus_darker)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Plus_lighter)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Blend_divide)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Blend_subtraction)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Darker_color)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Dissolve)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Hard_mix)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighter_color)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighten_burn)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighten_dodge)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighten_light)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Pass_through)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Pin_Light)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Vivid_light)
  SWITCH_MAP_ITEM_END(SkBlendMode::kSrcOver)
  return SkBlendMode::kSrcOver;
}

inline SkPaint::Cap toSkPaintCap(VGG::ELineCap cap)
{
  SWITCH_MAP_ITEM_BEGIN(cap)
  SWITCH_MAP_ITEM_DEF(LC_Butt, SkPaint::kButt_Cap)
  SWITCH_MAP_ITEM_DEF(LC_Round, SkPaint::kRound_Cap)
  SWITCH_MAP_ITEM_DEF(LC_Square, SkPaint::kSquare_Cap)
  SWITCH_MAP_ITEM_END(SkPaint::kButt_Cap)
}

inline skia::textlayout::TextAlign toSkTextAlign(ETextVerticalAlignment align)
{
  SWITCH_MAP_ITEM_BEGIN(align)
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_Top, skia::textlayout::TextAlign::kStart);
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_Bottom, skia::textlayout::TextAlign::kEnd);
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_Center, skia::textlayout::TextAlign::kCenter);
  SWITCH_MAP_ITEM_END(skia::textlayout::TextAlign::kStart)
}

inline skia::textlayout::TextAlign toSkTextAlign(ETextHorizontalAlignment align)
{
  SWITCH_MAP_ITEM_BEGIN(align)
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Left, skia::textlayout::TextAlign::kLeft);
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Right, skia::textlayout::TextAlign::kRight);
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Justify,
                      skia::textlayout::TextAlign::kJustify);
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Center,
                      skia::textlayout::TextAlign::kCenter);
  SWITCH_MAP_ITEM_DEF_NULL(VGG::ETextHorizontalAlignment::HA_Natural)
  SWITCH_MAP_ITEM_END(skia::textlayout::TextAlign::kLeft)
}

inline SkFontStyle toSkFontStyle(const std::string_view& subFamilyName)
{
  if (subFamilyName == "Bold")
  {
    return SkFontStyle::Bold();
  }
  else if (subFamilyName == "Regular")
  {
    return SkFontStyle::Normal();
  }
  else if (subFamilyName == "ExtraBold" || subFamilyName == "Extra Bold" ||
           subFamilyName == "Extra-Bold")
  {
    return SkFontStyle(SkFontStyle::kExtraBold_Weight,
                       SkFontStyle::kNormal_Width,
                       SkFontStyle::kUpright_Slant);
  }
  else if (subFamilyName == "Bold Italic" || subFamilyName == "BoldItalic" ||
           subFamilyName == "Bold-Italic")
  {
    return SkFontStyle::BoldItalic();
  }
  else
  {
    return SkFontStyle::Normal();
  }
}

inline SkPathOp toSkPathOp(VGG::EBoolOp blop)
{
  SWITCH_MAP_ITEM_BEGIN(blop)
  SWITCH_MAP_ITEM_DEF(VGG::BO_Union, SkPathOp::kUnion_SkPathOp)
  SWITCH_MAP_ITEM_DEF(BO_Substraction, SkPathOp::kDifference_SkPathOp)
  SWITCH_MAP_ITEM_DEF(VGG::BO_Intersection, SkPathOp::kIntersect_SkPathOp)
  SWITCH_MAP_ITEM_DEF(VGG::BO_Exclusion, SkPathOp::kXOR_SkPathOp)
  SWITCH_MAP_ITEM_END(SkPathOp::kUnion_SkPathOp)
}

constexpr float EPS = std::numeric_limits<float>::epsilon();

inline double calcRadius(double r0,
                         const glm::vec2& p0,
                         const glm::vec2& p1,
                         const glm::vec2& p2,
                         glm::vec2* left,
                         glm::vec2* right)
{
  glm::vec2 a = p0 - p1;
  glm::vec2 b = p2 - p1;
  double alen = glm::distance(p0, p1);
  double blen = glm::distance(p2, p1);
  if (std::fabs(alen) < EPS || std::fabs(blen) < EPS)
  {
    return 0.;
  }
  ASSERT(alen > 0 && blen > 0);
  double cosTheta = glm::dot(a, b) / alen / blen;
  if (cosTheta + 1 < EPS) // cosTheta == -1
  {
    if (left)
    {
      left->x = p1.x;
      left->y = p1.y;
    }
    if (right)
    {
      right->x = p1.x;
      right->y = p1.y;
    }
    return r0;
  }
  else if (1 - cosTheta < EPS) // cosTheta == 1
  {
    return 0.;
  }
  double tanHalfTheta = std::sqrt((1 - cosTheta) / (1 + cosTheta));
  double radius = r0;
  radius = std::min(radius, 0.5 * alen * tanHalfTheta);
  radius = std::min(radius, 0.5 * blen * tanHalfTheta);
  if (left)
  {
    ASSERT(tanHalfTheta > 0);
    float len = radius / tanHalfTheta;
    *left = p1 + float(len / alen) * a;
  }
  if (right)
  {
    ASSERT(tanHalfTheta > 0);
    double len = radius / tanHalfTheta;
    *right = p1 + (float(len / blen) * b);
  }
  return radius;
}

inline SkMatrix upperMatrix22(const SkMatrix& matrix)
{
  SkMatrix m = matrix;
  m.setTranslateX(0);
  m.setTranslateY(1);
  return m;
}

inline SkPath getSkiaPath(const std::vector<PointAttr>& points, bool isClosed)
{
  constexpr float W = 1.0;
  constexpr float H = 1.0;
  auto& pts = points;

  ASSERT(W > 0);
  ASSERT(H > 0);

  SkPath skPath;

  if (pts.size() < 2)
  {
    // WARN("Too few path points.");
    return skPath;
  }

  using PM = EPointMode;
  auto* startP = &pts[0];
  auto* endP = &pts[pts.size() - 1];
  auto* prevP = endP;
  auto* currP = startP;
  auto* nextP = currP + 1;

  const glm::vec2 s = { W, H };

  if (currP->radius > 0 && currP->mode() == PM::PM_Straight)
  {
    glm::vec2 start = currP->point * s;
    calcRadius(currP->radius,
               prevP->point * s,
               currP->point * s,
               nextP->point * s,
               nullptr,
               &start);
    skPath.moveTo(start.x, start.y);
  }
  else
  {
    skPath.moveTo(W * currP->point.x, H * currP->point.y);
  }

  while (true)
  {
    if (currP->mode() == PM::PM_Straight && nextP->mode() == PM::PM_Straight)
    {
      if (nextP->radius > 0 && nextP->mode() == PM::PM_Straight)
      {
        auto* next2P = (nextP == endP) ? startP : (nextP + 1);
        auto next2Pp = next2P->to.has_value() ? next2P->to.value() : next2P->point;
        double r = calcRadius(nextP->radius, currP->point * s, nextP->point * s, next2Pp * s, 0, 0);
        skPath.arcTo(W * nextP->point.x, H * nextP->point.y, W * next2Pp.x, H * next2Pp.y, r);
      }
      else
      {
        skPath.lineTo(W * nextP->point.x, H * nextP->point.y);
      }
    }
    else if (currP->mode() == PM::PM_Disconnected && nextP->mode() == PM::PM_Disconnected)
    {
      bool hasFrom = currP->from.has_value();
      bool hasTo = nextP->to.has_value();
      if (!hasFrom && !hasTo)
      {
        skPath.lineTo(W * nextP->point.x, H * nextP->point.y);
      }
      else if (hasFrom && !hasTo)
      {
        auto& from = currP->from.value();
        skPath.quadTo(W * from.x, H * from.y, W * nextP->point.x, H * nextP->point.y);
      }
      else if (!hasFrom && hasTo)
      {
        auto& to = nextP->to.value();
        skPath.quadTo(W * to.x, H * to.y, W * nextP->point.x, H * nextP->point.y);
      }
      else
      {
        auto& from = currP->from.value();
        auto& to = nextP->to.value();
        skPath.cubicTo(W * from.x,
                       H * from.y,
                       W * to.x,
                       H * to.y,
                       W * nextP->point.x,
                       H * nextP->point.y);
      }
    }
    else if (currP->mode() != PM::PM_Straight && nextP->mode() != PM::PM_Straight)
    {
      if ((currP->mode() == PM::PM_Disconnected && !currP->from.has_value()) ||
          (nextP->mode() == PM::PM_Disconnected && !nextP->to.has_value()) ||
          (currP->mode() != PM::PM_Disconnected &&
           !(currP->from.has_value() && currP->to.has_value())) ||
          (nextP->mode() != PM::PM_Disconnected &&
           !(nextP->from.has_value() && nextP->to.has_value())))
      {
        WARN("Missing control points.");
        return skPath;
      }
      auto& from = currP->from.value();
      auto& to = nextP->to.value();
      skPath.cubicTo(W * from.x,
                     H * from.y,
                     W * to.x,
                     H * to.y,
                     W * nextP->point.x,
                     H * nextP->point.y);
    }
    else if (currP->mode() == PM::PM_Straight && nextP->mode() != PM::PM_Straight)
    {
      if (!nextP->to.has_value())
      {
        skPath.lineTo(W * nextP->point.x, H * nextP->point.y);
      }
      else
      {
        auto& to = nextP->to.value();
        skPath.quadTo(W * to.x, H * to.y, W * nextP->point.x, H * nextP->point.y);
      }
    }
    else if (currP->mode() != PM::PM_Straight && nextP->mode() == PM::PM_Straight)
    {
      if (nextP->radius > 0 && nextP->mode() == PM::PM_Straight)
      {
        auto* next2P = (nextP == endP) ? startP : (nextP + 1);
        if (!currP->from.has_value())
        {
          glm::vec2 start;
          double r = calcRadius(nextP->radius,
                                currP->point * s,
                                nextP->point * s,
                                next2P->point * s,
                                &start,
                                nullptr);
          skPath.lineTo(start.x, start.y);
          skPath.arcTo(W * nextP->point.x,
                       H * nextP->point.y,
                       W * next2P->point.x,
                       H * next2P->point.y,
                       r);
        }
        else
        {
          auto currPfrom = currP->from.value();
          constexpr float RADIUS_COEFF = 1.0;
          // glm::vec2 p =
          glm::vec2 p = (currP->point + RADIUS_COEFF * (currPfrom - currP->point)) * s;
          glm::vec2 start;
          double r =
            calcRadius(nextP->radius, p, nextP->point * s, next2P->point * s, &start, nullptr);
          skPath.quadTo(p.x, p.y, start.x, start.y);
          skPath.arcTo(W * nextP->point.x,
                       H * nextP->point.y,
                       W * next2P->point.x,
                       H * next2P->point.y,
                       r);
        }
      }
      else
      {
        if (!currP->from.has_value())
        {
          skPath.lineTo(W * nextP->point.x, H * nextP->point.y);
        }
        else
        {
          auto& from = currP->from.value();
          skPath.quadTo(W * from.x, H * from.y, W * nextP->point.x, H * nextP->point.y);
        }
      }
    }
    else
    {
      WARN("Invalid point mode combination: %d %d", (int)currP->mode(), (int)nextP->mode());
    }
    currP = nextP;
    nextP = (nextP == endP) ? startP : (nextP + 1);

    if (isClosed)
    {
      if (currP == startP)
      {
        break;
      }
    }
    else
    {
      if (nextP == startP)
      {
        break;
      }
    }
  }

  if (isClosed)
  {
    skPath.close();
  }

  return skPath;
}

inline sk_sp<SkShader> getImageShader(sk_sp<SkImage> img,
                                      int width,
                                      int height,
                                      EImageFillType imageFillType,
                                      float imageTileScale,
                                      bool imageTileMirrored,
                                      const SkMatrix* matrix = nullptr)
{

  SkTileMode modeX = SkTileMode::kDecal;
  SkTileMode modeY = SkTileMode::kDecal;
  SkMatrix mat = SkMatrix::I();
  if (matrix)
  {
    mat.postConcat(*matrix);
  }
  SkImageInfo mi = img->imageInfo();
  float sx = (float)width / mi.width();
  float sy = (float)height / mi.height();
  if (imageFillType == IFT_Fill)
  {
    double s = std::max(sx, sy);
    mat.postScale(s, s);
    if (matrix)
    {
      mat.postConcat(upperMatrix22(*matrix));
    }
    if (sx > sy)
    {
      // scaled image's width == frame's width
      mat.postTranslate(0, (height - sx * mi.height()) / 2);
    }
    else
    {
      // scaled image's height == frame's height
      mat.postTranslate((width - sy * mi.width()) / 2, 0);
    }
  }
  else if (imageFillType == IFT_Fit)
  {

    if (matrix)
    {
      mat.postConcat(upperMatrix22(*matrix));
    }
    double s = std::min(sx, sy);
    mat.postScale(s, s);
    if (matrix)
    {
      mat.postConcat(*matrix);
    }
    if (sx < sy)
    {
      // scaled image's width == frame's width
      mat.postTranslate(0, (height - sx * mi.height()) / 2);
    }
    else
    {
      // scaled image's height == frame's height
      mat.postTranslate((width - sy * mi.width()) / 2, 0);
    }
  }
  else if (imageFillType == VGG::IFT_Stretch)
  {
    if (matrix)
    {
      mat.postConcat(upperMatrix22(*matrix));
    }
    mat.postScale(sx, sy);
  }
  else if (imageFillType == VGG::IFT_Tile)
  {
    modeX = imageTileMirrored ? SkTileMode::kMirror : SkTileMode::kRepeat;
    modeY = imageTileMirrored ? SkTileMode::kMirror : SkTileMode::kRepeat;
    if (matrix)
    {
      mat.postConcat(*matrix);
    }
  }
  else if (imageFillType == IFT_OnlyTileVertical)
  {
    if (matrix)
    {
      mat.postConcat(*matrix);
    }
    modeY = imageTileMirrored ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  else if (imageFillType == IFT_OnlyTileHorizontal)
  {
    modeX = imageTileMirrored ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  SkSamplingOptions opt;
  mat.postScale(1, -1); // convert to skia

  return img->makeShader(modeX, modeY, opt, &mat);
}

inline sk_sp<SkImage> loadImage(const std::string& imageGUID, const ResourceRepo& repo)
{
  sk_sp<SkImage> image;
  if (imageGUID.empty())
    return image;
  std::string guid = imageGUID;
  if (auto pos = guid.find("./"); pos != std::string::npos && pos == 0)
  {
    // remove current dir notation
    guid = guid.substr(2);
  }
  if (auto it = g_skiaImageRepo.find(guid); it != g_skiaImageRepo.end())
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
        return image;
      }
      sk_sp<SkImage> skImage = SkImages::DeferredFromEncodedData(data);
      if (!skImage)
      {
        WARN("Make SkImage failed.");
        return image;
      }
      g_skiaImageRepo[guid] = skImage;
      image = skImage;
    }
    else
    {
      WARN("Cannot find %s from resources repository", guid.c_str());
    }
  }
  return image;
}

inline sk_sp<SkImageFilter> makeBlendModeFilter(EBlendMode blendMode)
{
  switch (blendMode)
  {
    case VGG::BM_Blend_divide:
      SkImageFilters::Arithmetic(0, 0, 0, 0, true, nullptr, nullptr);
    default:
      return nullptr;
  }
}

inline SkRect toSkRect(const VGG::Bound2& bound)
{
  const auto& b = bound;
  return SkRect{ b.topLeft.x, b.topLeft.y, b.bottomRight.x, b.bottomRight.y };
}

inline SkMatrix toSkMatrix(const glm::mat3& mat)
{
  SkMatrix skMatrix;
  skMatrix.setAll(mat[0][0],
                  mat[1][0],
                  mat[2][0],
                  mat[0][1],
                  mat[1][1],
                  mat[2][1],
                  mat[0][2],
                  mat[1][2],
                  mat[2][2]);
  return skMatrix;
}

inline float calcRotationAngle(const glm::mat3& mat)
{
  glm::vec3 v{ 1, 0, 0 };
  auto r = mat * v;
  const auto v1 = glm::vec2{ r.x, r.y };
  auto d = glm::dot(glm::vec2{ 1, 0 }, v1);
  float cosAngle = d / glm::length(v1);
  float radian = acos(cosAngle);
  return radian * 180.0 / 3.141592653;
}

inline float calcRotationAngle(const SkMatrix& mat)
{
  SkVector r;
  mat.mapVector(1, 0, &r);
  float d = r.dot({ 1, 0 });
  float cosAngle = r.dot({ 1, 0 }) / r.length();
  float radian = acos(cosAngle);
  return radian * 180.0 / 3.141592653;
}

inline SkColor nodeType2Color(ObjectType type)
{
  switch (type)
  {
    case ObjectType::VGG_PATH:
      return SK_ColorRED;
    case ObjectType::VGG_IMAGE:
      return SK_ColorRED;
    case ObjectType::VGG_GROUP:
      return SK_ColorRED;
    case ObjectType::VGG_TEXT:
      return SK_ColorRED;
    case ObjectType::VGG_ARTBOARD:
      return SK_ColorRED;
    case ObjectType::VGG_LAYER:
      return SK_ColorRED;
    case ObjectType::VGG_MASTER:
      return SK_ColorRED;
    case ObjectType::VGG_CONTOUR:
      return SK_ColorYELLOW;
    default:
      return SK_ColorRED;
  }
}
