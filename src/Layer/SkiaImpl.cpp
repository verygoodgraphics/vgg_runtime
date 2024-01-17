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
#include "VSkia.hpp"
#include <core/SkM44.h>
#include <effects/SkRuntimeEffect.h>

// TODO:: refactor as LRU cache and make it non global
std::unordered_map<std::string, sk_sp<SkImage>> g_skiaImageRepo = {};

sk_sp<SkColorFilter> makeColorFilter(const ImageFilter& imageFilter)
{
  if (imageFilter.isDefault())
    return nullptr;
  static const char* s_sksl = R"(
uniform float exposure;
uniform float contrast;
uniform float saturation;
uniform float temperature;
uniform float tint;
uniform float hightlight;
uniform float shadow;
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
vec4 main(vec4 inColor){
    return pow(2, exposure) * saturationMatrix(saturation) * contrastMatrix(contrast) * inColor;
}
)";

  auto fx = GetOrCreateEffect("imageAdjust", s_sksl);
  if (fx)
  {
    SkRuntimeColorFilterBuilder builder(fx);
    builder.uniform("exposure") = imageFilter.exposure;
    builder.uniform("contrast") = 1 + imageFilter.contrast;
    builder.uniform("saturation") = 1 + imageFilter.saturation;
    builder.uniform("temperature") = imageFilter.temperature;
    builder.uniform("tint") = imageFilter.tint;
    builder.uniform("hightlight") = imageFilter.highlight;
    builder.uniform("shadow") = imageFilter.shadow;
    return builder.makeColorFilter();
  }
  return nullptr;
}
namespace VGG::layer
{

}; // namespace VGG::layer
