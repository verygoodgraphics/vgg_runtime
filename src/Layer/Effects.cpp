/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Layer/LayerCache.h"
#include <effects/SkRuntimeEffect.h>
#include <core/SkM44.h>

namespace VGG::layer
{
// NOLINTBEGIN
sk_sp<SkBlender> getOrCreateBlender(EffectCacheKey name, const char* sksl)
{
  auto cache = GlobalBlenderCache();
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
  auto cache = GlobalEffectCache();
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
// NOLINTEND

} // namespace VGG::layer

namespace
{
SkSamplingOptions g_globalSamplingOption{};
} // namespace

namespace VGG::layer
{
void SetGlobalSamplingOptions(const SkSamplingOptions& opt)
{
  g_globalSamplingOption = opt;
}

const SkSamplingOptions& GlobalSamplingOptions()
{
  return g_globalSamplingOption;
}

// NOLINTEND
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

sk_sp<SkShader> makeFitPattern(const Bound& bound, const PatternFit& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());
  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bound.width();
  float       height = bound.height();
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
  auto       shader = img->makeShader(modeX, modeY, GlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkShader> makeFillPattern(const Bound& bound, const PatternFill& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());

  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bound.width();
  float       height = bound.height();
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
  auto       shader = img->makeShader(modeX, modeY, GlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkShader> makeStretchPattern(const Bound& bound, const PatternStretch& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());
  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bound.width();
  float       height = bound.height();
  auto        m = glm::mat3{ 1.0 };
  m = glm::scale(m, { width, height });
  m *= p.transform.matrix();
  m = glm::scale(m, { 1.f / mi.width(), 1.f / mi.height() });
  const auto mat = toSkMatrix(m);
  SkTileMode modeX = SkTileMode::kDecal;
  SkTileMode modeY = SkTileMode::kDecal;
  auto       shader = img->makeShader(modeX, modeY, GlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkShader> makeTilePattern(const Bound& bound, const PatternTile& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());

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
  auto       shader = img->makeShader(modeX, modeY, GlobalSamplingOptions(), &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

sk_sp<SkShader> makeGradientLinear(const Bound& bound, const GradientLinear& g)
{
  if (g.stops.empty())
    return nullptr;
  auto minPosition = g.stops.front().position;
  auto maxPosition = g.stops.back().position;
  // clampPairByLimits(minPosition, maxPosition, 0.f, 1.f, 0.0001f);

  auto f = bound.map(bound.size() * g.from);
  auto t = bound.map(bound.size() * g.to);
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

} // namespace VGG::layer
