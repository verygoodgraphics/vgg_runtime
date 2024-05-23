/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Effects.hpp"
#include "Layer/PenNode.hpp"
#include "Renderer.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/LayerCache.h"
#include "Layer/ShapeAttribute.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/Core/VShape.hpp"
#include <core/SkBlendMode.h>
#include <core/SkColor.h>
#include <core/SkCanvas.h>
#include <effects/SkRuntimeEffect.h>
#include <core/SkM44.h>
#include <src/core/SkBlurMask.h>

namespace VGG::layer
{
sk_sp<SkBlender> getOrCreateBlender(EffectCacheKey name, const char* sksl)
{
  auto cache = getGlobalBlenderCache();
  auto b = cache->find(name);
  if (!b)
  {
    auto result = SkRuntimeEffect::MakeForBlender(SkString(sksl));
    if (!result.effect)
    {
      DEBUG("Runtime Effect Failed[%s]: %s", name, result.errorText.data());
      return nullptr;
    }
    auto blender = result.effect->makeBlender(nullptr);
    return *cache->insert(name, std::move(blender));
  }
  return *b;
}

sk_sp<SkRuntimeEffect> getOrCreateEffect(EffectCacheKey key, const char* sksl)
{
  auto cache = getGlobalEffectCache();
  auto b = cache->find(key);
  if (!b)
  {
    auto result = SkRuntimeEffect::MakeForColorFilter(SkString(sksl));
    if (!result.effect)
    {
      DEBUG("Runtime Effect Failed[%s]: %s", key, result.errorText.data());
      return nullptr;
    }
    return *cache->insert(key, std::move(result.effect));
  }
  return *b;
}

} // namespace VGG::layer

namespace
{
SkSamplingOptions g_globalSamplingOption{};
} // namespace

namespace VGG::layer
{
void setGlobalSamplingOptions(const SkSamplingOptions& opt)
{
  g_globalSamplingOption = opt;
}

const SkSamplingOptions& getGlobalSamplingOptions()
{
  return g_globalSamplingOption;
}

sk_sp<SkColorFilter> makeColorFilter(const ImageFilter& imageFilter)
{
  if (imageFilter.isDefault())
    return nullptr;
  static const char* s_sksl = R"(
uniform float exposure;
uniform float contrast;
uniform float saturation;
uniform float temperature;
uniform float highlight;
uniform float shadow;
uniform float tint;
uniform float hue;
uniform float3 tintColor1;
uniform float3 tintColor2;
vec4 changeHue(vec4 color, float H){
    const vec4  kRGBToYPrime = vec4 (0.299, 0.587, 0.114, 0.0);
    const vec4  kRGBToI     = vec4 (0.596, -0.275, -0.321, 0.0);
    const vec4  kRGBToQ     = vec4 (0.212, -0.523, 0.311, 0.0);
    const vec4  kYIQToR   = vec4 (1.0, 0.956, 0.621, 0.0);
    const vec4  kYIQToG   = vec4 (1.0, -0.272, -0.647, 0.0);
    const vec4  kYIQToB   = vec4 (1.0, -1.107, 1.704, 0.0);
    // Convert to YIQ
    float   YPrime  = dot (color, kRGBToYPrime);
    float   I      = dot (color, kRGBToI);
    float   Q      = dot (color, kRGBToQ);

    // Calculate the hue and chroma
    float   hue     = atan (Q, I);
    float   chroma  = sqrt (I * I + Q * Q);

    // Make the user's adjustments
    hue += H * 3.1415926535;

    // Convert back to YIQ
    Q = chroma * sin (hue);
    I = chroma * cos (hue);

    // Convert back to RGB
    vec4    yIQ   = vec4 (YPrime, I, Q, 0.0);
    color.r = dot (yIQ, kYIQToR);
    color.g = dot (yIQ, kYIQToG);
    color.b = dot (yIQ, kYIQToB);
    return color;
}

mat4 brightnessMatrix( float brightness )
{
    return mat4( 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 brightness, brightness, brightness, 1 );
}
mat4 contrastMatrix( float contrast )
{
    float t = ( 1.0 - contrast ) / 2.0;
    return mat4( contrast, 0, 0, 0,
                 0, contrast, 0, 0,
                 0, 0, contrast, 0,
                 t, t, t, 1 );

}

mat4 saturationMatrix( float saturation )
{
    //vec3 luminance = vec3( 0.3086, 0.6094, 0.0820 );
    vec3 luminance = vec3( 0.213, 0.715, 0.0720 );
    float oneMinusSat = 1.0 - saturation;
    vec3 red = vec3( luminance.x * oneMinusSat );
    red+= vec3( saturation, 0, 0 );
    vec3 green = vec3( luminance.y * oneMinusSat );
    green += vec3( 0, saturation, 0 );
    vec3 blue = vec3( luminance.z * oneMinusSat );
    blue += vec3( 0, 0, saturation );
    return mat4( red,     0,
                 green,   0,
                 blue,    0,
                 0, 0, 0, 1 );
}

mat4 temperatureMatrix(float tem){
    mat4 temperatureMatrix = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    if (tem > 0.0) {
        temperatureMatrix[0][0] += tem;  // red
        temperatureMatrix[2][2] -= tem;  // blue
    } else {
        temperatureMatrix[0][0] += tem;
        temperatureMatrix[2][2] -= tem;
    }
    return temperatureMatrix;
}

mat4 tintMatrix(float tint, vec3 tintColor1, vec3 tintColor2){
    vec3 tc = vec3(1,1,1);
    if(tint > 0){
       tc = tintColor1 * tint;
    }else if(tint < 0){
       tc = tintColor2 * -tint;
    }
    return mat4(
        tc.r, 0.0, 0.0, 0.0,
        0.0, tc.g, 0.0, 0.0,
        0.0, 0.0, tc.b, 0.0,
        0.0, 0.0, 0.0, 1.0);
}

vec3 highlightAndShadow(vec3 color){
    float lumR = 0.299;
    float lumG = 0.587;
    float lumB = 0.114;
    vec3 luminance = sqrt(vec3(lumR,lumG,lumB)*pow(color,vec3(2,2,2)));
    vec3 h = highlight * 0.05 * (pow(vec3(8,8,8), luminance) - 1.0);
    vec3 s = shadow * 0.05 * (pow(vec3(8,8,8), 1.0 - luminance) - 1.0);
    return color + h + s;
}

vec4 main(vec4 inColor){
    vec4 color = pow(3, exposure) * 
                 contrastMatrix(contrast) *
                 saturationMatrix(saturation) *
                 temperatureMatrix(temperature * 0.25) *
                 tintMatrix(tint, tintColor1, tintColor2) *
                 vec4(highlightAndShadow(inColor.rgb),1.0);
    color =  changeHue(color, -hue);
    return vec4(color.rgb, inColor.a);
}
)";

  auto fx = getOrCreateEffect("imageFilters", s_sksl);
  if (fx)
  {
    SkRuntimeColorFilterBuilder builder(fx);
    builder.uniform("exposure") = imageFilter.exposure;
    builder.uniform("contrast") = 1 + imageFilter.contrast;
    builder.uniform("saturation") = 1 + imageFilter.saturation;
    builder.uniform("temperature") = imageFilter.temperature;
    builder.uniform("highlight") = imageFilter.highlight;
    builder.uniform("shadow") = imageFilter.shadow;
    builder.uniform("tint") = imageFilter.tint;
    builder.uniform("tintColor1") = SkV3{ 1.f, 0.9f, 0.2f };
    builder.uniform("tintColor2") = SkV3{ 1.f, 0.5f, 0.9f };
    builder.uniform("hue") = imageFilter.hue;
    return builder.makeColorFilter();
  }
  return nullptr;
}

sk_sp<SkShader> makeFitPattern(const Bounds& bounds, const PatternFit& p)
{
  auto img = loadImage(p.guid);
  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bounds.width();
  float       height = bounds.height();
  float       sx = (float)width / mi.width();
  float       sy = (float)height / mi.height();
  auto        m = glm::mat3{ 1.0 };
  float       s = std::min(sx, sy);
  if (sx < sy)
  {
    m = glm::translate(m, { 0, (height - s * mi.height()) / 2 });
  }
  else
  {
    m = glm::translate(m, { (width - s * mi.width()) / 2, 0 });
  }
  m = glm::scale(m, { s, s });
  m = glm::translate(m, { mi.width() / 2, mi.height() / 2 });
  m = glm::rotate(m, p.rotation);
  m = glm::translate(m, { -mi.width() / 2, -mi.height() / 2 });
  SkTileMode modeX = SkTileMode::kDecal;
  SkTileMode modeY = SkTileMode::kDecal;
  const auto mat = toSkMatrix(m);
  auto       shader = img->makeShader(modeX, modeY, getGlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkShader> makeFillPattern(const Bounds& bounds, const PatternFill& p)
{
  auto img = loadImage(p.guid);

  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bounds.width();
  float       height = bounds.height();
  float       sx = (float)width / mi.width();
  float       sy = (float)height / mi.height();
  auto        m = glm::mat3{ 1.0 };
  const float s = std::max(sx, sy);
  if (sx > sy)
  {
    m = glm::translate(m, { 0, (height - s * mi.height()) / 2.f });
  }
  else
  {
    m = glm::translate(m, { (width - s * mi.width()) / 2.f, 0 });
  }
  m = glm::scale(m, { s, s });
  m = glm::translate(m, { mi.width() / 2, mi.height() / 2 });
  m = glm::rotate(m, p.rotation);
  m = glm::translate(m, { -mi.width() / 2, -mi.height() / 2 });
  SkTileMode modeX = SkTileMode::kDecal;
  SkTileMode modeY = SkTileMode::kDecal;
  const auto mat = toSkMatrix(m);
  auto       shader = img->makeShader(modeX, modeY, getGlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkShader> makeStretchPattern(const Bounds& bounds, const PatternStretch& p)
{
  auto img = loadImage(p.guid);
  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bounds.width();
  float       height = bounds.height();
  auto        m = glm::mat3{ 1.0 };
  m = glm::scale(m, { width, height });
  m *= p.transform.matrix();
  m = glm::scale(m, { 1.f / mi.width(), 1.f / mi.height() });
  const auto mat = toSkMatrix(m);
  SkTileMode modeX = SkTileMode::kDecal;
  SkTileMode modeY = SkTileMode::kDecal;
  auto       shader = img->makeShader(modeX, modeY, getGlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkImageFilter> makeMotionBlurFilter(const MotionBlur& blur)
{
  auto result = SkRuntimeEffect::MakeForShader(SkString(R"(
uniform shader child;
uniform float radius;
uniform float angle;

const float HASHSCALE1 = 443.8975;
const int SAMPLE = 10;
float hash13(vec3 p3)
{
    p3 = fract(p3 * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

half4 main(float2 coord){
    half4 color = vec4(0, 0, 0, 0);
    float2 d = float2(cos(angle), sin(angle));
    for(int i = 0;i < SAMPLE ; i++){
        float rnd = hash13(vec3(coord.x, coord.y, float(i)));
        float t = (float(i) + rnd) / float(SAMPLE); 
        t = (t * 2.0 - 1.0) * radius;
        float2 p = coord + d * t;
        color += child.eval(p);
    }
    color /= float(SAMPLE);
    return color;
}
    )"));
  if (result.effect == nullptr)
  {
    DEBUG("Runtime Effect Failed[%s]: %s", "motion blur", result.errorText.data());
    return nullptr;
  }
  SkRuntimeShaderBuilder builder(std::move(result.effect));
  builder.uniform("angle") = SkScalar(blur.angle) * (float)M_PI / 180.f;
  builder.uniform("radius") = blur.radius;
  return SkImageFilters::RuntimeShader(builder, blur.radius, "", nullptr);
}

sk_sp<SkImageFilter> makeRadialBlurFilter(const RadialBlur& blur, const Bounds& bounds)
{
  auto result = SkRuntimeEffect::MakeForShader(SkString(R"(
uniform shader child;
uniform float radius;
uniform float2 center;

const float HASHSCALE1 = 443.8975;
const int SAMPLE = 10;
float hash13(vec3 p3)
{
    p3 = fract(p3 * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

half4 main(float2 coord){
    half4 color = vec4(0, 0, 0, 0);
    float2 d = normalize(center - coord);
    for(int i = 0;i < SAMPLE ; i++){
        float rnd = hash13(vec3(coord.x, coord.y, float(i)));
        float t = (float(i) + rnd) / float(SAMPLE); 
        t = (t * 2.0 - 1.0) * radius;
        float2 p = coord + d * t;
        color += child.eval(p);
    }
    color /= float(SAMPLE);
    return color;
}
)"));
  if (result.effect == nullptr)
  {
    DEBUG("Runtime Effect Failed[%s]: %s", "motion blur", result.errorText.data());
    return nullptr;
  }
  SkRuntimeShaderBuilder builder(std::move(result.effect));
  builder.uniform("radius") = blur.radius;
  builder.uniform("center") = SkV2{ blur.xCenter * bounds.width(), blur.yCenter * bounds.height() };
  return SkImageFilters::RuntimeShader(builder, blur.radius, "", nullptr);
}
sk_sp<SkImageFilter> makeLayerBlurFilter(const GaussianBlur& blur)
{

  return SkImageFilters::Blur(
    SkBlurMask::ConvertRadiusToSigma(blur.radius),
    SkBlurMask::ConvertRadiusToSigma(blur.radius),
    0);
}

sk_sp<SkImageFilter> makeBackgroundBlurFilter(const GaussianBlur& blur)
{
  return SkImageFilters::Blur(
    SkBlurMask::ConvertRadiusToSigma(blur.radius),
    SkBlurMask::ConvertRadiusToSigma(blur.radius),
    0);
}

sk_sp<SkShader> makeTilePattern(const Bounds& bounds, const PatternTile& p)
{
  auto img = loadImage(p.guid);

  if (!img)
    return nullptr;
  SkTileMode modeX = SkTileMode::kDecal;
  SkTileMode modeY = SkTileMode::kDecal;
  if (p.mode == TILE_VERTICAL)
  {
    modeY = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  if (p.mode == TILE_HORIZONTAL)
  {
    modeX = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  if (p.mode == TILE_BOTH)
  {
    modeY = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
    modeX = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  auto m = glm::mat3{ 1.0 };
  m = glm::rotate(m, p.rotation);
  m = glm::scale(m, { p.scale, p.scale });
  const auto mat = toSkMatrix(m);
  auto       shader = img->makeShader(modeX, modeY, getGlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkShader> makeGradientLinear(const Bounds& bounds, const GradientLinear& g)
{
  if (g.stops.empty())
    return nullptr;
  auto minPosition = g.stops.front().position;
  auto maxPosition = g.stops.back().position;
  auto f = bounds.map(bounds.size() * g.from);
  auto t = bounds.map(bounds.size() * g.to);
  auto start = glm::mix(f, t, minPosition);
  auto end = glm::mix(f, t, maxPosition);

  SkPoint pts[2] = {
    { (SkScalar)start.x, (SkScalar)start.y },
    { (SkScalar)end.x, (SkScalar)end.y },
  };
  std::vector<SkColor>  colors;
  std::vector<SkScalar> positions;
  for (auto it = g.stops.begin(); it != g.stops.end(); ++it)
  {
    colors.push_back(it->color);
    auto p = it->position;
    positions.push_back((p - minPosition) / (maxPosition - minPosition));
  }
  SkMatrix mat = SkMatrix::I();
  auto     s = SkGradientShader::MakeLinear(
    pts,
    colors.data(),
    positions.data(),
    colors.size(),
    SkTileMode::kClamp,
    0,
    &mat);
  return s;
}

sk_sp<SkImageFilter> makeInnerShadowImageFilter(
  const InnerShadow&   shadow,
  const Bounds&        bounds,
  bool                 shadowOnly,
  bool                 overrideSpread,
  sk_sp<SkImageFilter> input)
{
  auto sigma = SkBlurMask::ConvertRadiusToSigma(shadow.blur);
  auto alpha =
    SkImageFilters::ColorFilter(SkColorFilters::Blend(SK_ColorBLACK, SkBlendMode::kSrcIn), 0);

  // auto blender = getOrCreateBlender("innerShadow", R"(
  // vec4 main(vec4 srcColor, vec4 dstColor) {
  // return vec4(0,0,0,step(0.5, srcColor.a));
  // }
  // )");
  //
  // auto alpha = SkImageFilters::Blend(blender, 0, 0);
  auto f1 =
    SkImageFilters::ColorFilter(SkColorFilters::Blend(shadow.color, SkBlendMode::kSrcOut), 0);
  sk_sp<SkImageFilter> f2;
  if (overrideSpread || shadow.spread == 0)
  {
    f2 = SkImageFilters::Offset(shadow.offsetX, shadow.offsetY, f1);
  }
  else
  {
    const auto     halfWidth = bounds.width() / 2;
    const auto     halfHeight = bounds.height() / 2;
    const float    scaleX = std::max((bounds.width() - 2 * shadow.spread) / bounds.width(), 0.f);
    const float    scaleY = std::max((bounds.height() - 2 * shadow.spread) / bounds.height(), 0.f);
    const SkVector c = { halfWidth - halfWidth * scaleX, halfHeight - halfHeight * scaleY };
    SkMatrix       m = SkMatrix::I();
    m.postScale(scaleX, scaleY);
    m.postTranslate(shadow.offsetX + c.fX, shadow.offsetY + c.fY);
    f2 = SkImageFilters::MatrixTransform(m, getGlobalSamplingOptions(), f1);
  }
  auto f3 = SkImageFilters::Blur(sigma, sigma, SkTileMode::kDecal, f2);
  auto f4 = SkImageFilters::Blend(SkBlendMode::kSrcIn, alpha, f3);
  // auto f4 = f3;
  if (shadowOnly)
  {
    return f4;
  }
  auto src =
    SkImageFilters::ColorFilter(SkColorFilters::Blend(SK_ColorBLACK, SkBlendMode::kDst), 0);
  return SkImageFilters::Compose(input, SkImageFilters::Blend(SkBlendMode::kSrcOver, src, f4));
}

sk_sp<SkImageFilter> makeDropShadowImageFilter(
  const DropShadow&    shadow,
  const Bounds&        bounds,
  bool                 overrideSpread,
  sk_sp<SkImageFilter> input)
{
  auto sigma = SkBlurMask::ConvertRadiusToSigma(shadow.blur);
  if (overrideSpread || shadow.spread == 0.f)
  {
    return SkImageFilters::DropShadowOnly(
      shadow.offsetX,
      shadow.offsetY,
      sigma,
      sigma,
      shadow.color,
      input);
  }
  const auto     halfWidth = bounds.width() / 2;
  const auto     halfHeight = bounds.height() / 2;
  const float    scaleX = std::max((bounds.width() + 2 * shadow.spread) / bounds.width(), 0.f);
  const float    scaleY = std::max((bounds.height() + 2 * shadow.spread) / bounds.height(), 0.f);
  const SkVector c = { halfWidth - halfWidth * scaleX, halfHeight - halfHeight * scaleY };
  SkMatrix       m = SkMatrix::I();
  m.postScale(scaleX, scaleY);
  m.postTranslate(shadow.offsetX + c.fX, shadow.offsetY + c.fY);
  auto s = SkImageFilters::DropShadowOnly(0, 0, sigma, sigma, shadow.color, input);
  return SkImageFilters::MatrixTransform(m, getGlobalSamplingOptions(), s);
}

SkRect drawBorder(
  Renderer*                  renderer,
  const VShape&              border,
  const SkRect&              bounds,
  const std::vector<Border>& borders,
  sk_sp<SkBlender>           blender)
{
  SkRect     resultBounds = bounds;
  const auto shapeBounds = resultBounds;
  for (const auto& b : borders)
  {
    if (!b.isEnabled || b.thickness <= 0)
      continue;

    SkPaint strokePen;
    strokePen.setAntiAlias(true);
    strokePen.setBlender(blender);
    // strokePen.setImageFilter(imageFilter);
    populateSkPaint(b, shapeBounds, strokePen);

    bool  inCenter = true;
    float strokeWidth = b.thickness;
    if (b.position == PP_INSIDE && border.isClosed())
    {
      // inside
      strokeWidth = 2.f * b.thickness;
      renderer->canvas()->save();
      border.clip(renderer->canvas(), SkClipOp::kIntersect);
      inCenter = false;
    }
    else if (b.position == PP_OUTSIDE && border.isClosed())
    {
      // outside
      strokeWidth = 2.f * b.thickness;
      renderer->canvas()->save();
      border.clip(renderer->canvas(), SkClipOp::kDifference);
      inCenter = false;
    }
    strokePen.setStrokeWidth(strokeWidth);
    border.draw(renderer->canvas(), strokePen);
    SkRect borderBounds;
    strokePen.computeFastBounds(shapeBounds, &borderBounds);
    resultBounds.join(borderBounds);
    if (!inCenter)
    {
      renderer->canvas()->restore();
    }
  }

  if (false)
  {
    auto                 pt = border.asPath();
    std::vector<SkPoint> pts(pt.countPoints());
    SkPaint              p;
    pt.getPoints(pts.data(), pts.size());
    p.setStrokeWidth(2);
    p.setColor(SK_ColorRED);
    SkFont a;
    renderer->canvas()->drawPoints(SkCanvas::kPoints_PointMode, pts.size(), pts.data(), p);
    for (std::size_t i = 0; i < pts.size(); i++)
    {
      SkPaint textPaint;
      textPaint.setStrokeWidth(0.5);
      textPaint.setColor(SK_ColorBLACK);
      std::string index = std::to_string(i);
      renderer->canvas()->drawSimpleText(
        index.c_str(),
        index.size(),
        SkTextEncoding::kUTF8,
        pts[i].x(),
        pts[i].y(),
        a,
        textPaint);
    }
  }
  return resultBounds;
}

void GraphicItemEffectNode::render(Renderer* renderer)
{
  if (auto s = shape(); s)
  {
    if (auto vs = s->getShape(); !vs.isEmpty())
    {
      onRenderShape(renderer, vs);
    }
  }
}

// StackFillEffectImpl begin
void StackFillEffectImpl::onRenderShape(Renderer* renderer, const VShape& vs)
{
  for (const auto& p : m_pens)
  {
    // auto r = vs.bounds();
    // auto bounds = Bounds{ r.x(), r.y(), r.width(), r.height() };
    auto paint = p->paint(bounds());
    vs.draw(renderer->canvas(), paint);
  }
}

void StackFillEffectImpl::setFillStyle(std::vector<Fill> fills)
{
  if (!changed(fills))
    return;

  for (auto& pen : m_pens)
  {
    unobserve(pen);
  }
  m_pens.clear();

  for (auto& fill : fills)
  {
    auto pen = FillPenNode::Make(fill);
    observe(pen);
    m_pens.push_back(pen);
  }
  invalidate();
}
// StackFillEffectImpl end

// StackBorderEffectImpl begin
void StackBorderEffectImpl::onRenderShape(Renderer* renderer, const VShape& border)
{
  SkRect     resultBounds = border.bounds();
  const auto shapeBounds = resultBounds;
  auto       originalBounds =
    Bounds{ resultBounds.x(), resultBounds.y(), resultBounds.width(), resultBounds.height() };
  for (const auto& p : m_pens)
  {
    auto b = p->getBorder();
    if (b.isEnabled && b.thickness > 0)
    {
      auto strokePen = p->paint(originalBounds); // TODO:: we can use effectBounds for more accurate

      bool  inCenter = true;
      float strokeWidth = b.thickness;
      if (b.position == PP_INSIDE && border.isClosed())
      {
        // inside
        strokeWidth = 2.f * b.thickness;
        renderer->canvas()->save();
        border.clip(renderer->canvas(), SkClipOp::kIntersect);
        inCenter = false;
      }
      else if (b.position == PP_OUTSIDE && border.isClosed())
      {
        // outside
        strokeWidth = 2.f * b.thickness;
        renderer->canvas()->save();
        border.clip(renderer->canvas(), SkClipOp::kDifference);
        inCenter = false;
      }
      strokePen.setStrokeWidth(strokeWidth);

      border.draw(renderer->canvas(), strokePen);

      SkRect borderBounds;
      strokePen.computeFastBounds(shapeBounds, &borderBounds);
      resultBounds.join(borderBounds);
      if (!inCenter)
      {
        renderer->canvas()->restore();
      }
    }
  }
}

void StackBorderEffectImpl::setBorderStyle(std::vector<Border> borders)
{
  if (!changed(borders))
    return;
  for (auto& pen : m_pens)
  {
    unobserve(pen);
  }
  m_pens.clear();
  for (auto& border : borders)
  {
    auto pen = BorderPenNode::Make(border);
    observe(pen);
    m_pens.push_back(pen);
  }
  invalidate();
}

Bounds StackFillEffectImpl::onRevalidate()
{
  for (const auto& p : m_pens)
  {
    p->revalidate();
  }
  return getChild()->bounds();
}

// StackBorderEffectImpl end

// Bounds InnerShadowEffectImpl::onRevalidate()
// {
//   return Bounds();
// }
//
// Ref<InnerShadowEffectImpl> InnerShadowEffectImpl::MakeFrom(
//   Ref<RenderNode>                 child,
//   const std::vector<InnerShadow>& shadows)
// {
//   ASSERT(!shadows.empty());
//   ASSERT(child);
//   auto first = InnerShadowEffectImpl::Make(child, shadows[0]);
//   for (size_t i = 1; i < shadows.size(); ++i)
//   {
//     first = InnerShadowEffectImpl::Make(first, shadows[i]);
//   }
//   return first;
// }
//
// Bounds DropShadowEffectImpl::onRevalidate()
// {
//   return Bounds();
// }
//
// Ref<DropShadowEffectImpl> DropShadowEffectImpl::MakeFrom(
//   Ref<RenderNode>                child,
//   const std::vector<DropShadow>& shadows)
// {
//   ASSERT(!shadows.empty());
//   ASSERT(child);
//   auto first = DropShadowEffectImpl::Make(child, shadows[0]);
//   for (size_t i = 1; i < shadows.size(); ++i)
//   {
//     first = DropShadowEffectImpl::Make(first, shadows[i]);
//   }
//   return first;
// }
} // namespace VGG::layer
