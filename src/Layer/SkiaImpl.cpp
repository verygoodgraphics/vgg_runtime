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
    vec3 h = highlight * 0.05 * ( pow(vec3(8,8,8), luminance) - 1.0 );
    vec3 s = shadow * 0.05 * ( pow(vec3(8,8,8), 1.0 - luminance) - 1.0 );
    return color + h + s;
}

vec4 main(vec4 inColor){
    vec3 color =(pow(3, exposure) * tintMatrix(tint, tintColor1, tintColor2) *
                temperatureMatrix(temperature *0.5) *
                saturationMatrix(saturation) *
                contrastMatrix(contrast) * inColor).rgb;
    return vec4(highlightAndShadow(color), inColor.a);
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
    builder.uniform("highlight") = imageFilter.highlight;
    builder.uniform("shadow") = imageFilter.shadow;
    builder.uniform("tint") = imageFilter.tint;
    builder.uniform("tintColor1") = SkV3{ 1.f, 0.9f, 0.2f };
    builder.uniform("tintColor2") = SkV3{ 1.f, 0.5f, 0.9f };
    builder.uniform("hue") = imageFilter.hue;
    DEBUG(
      "makeColorFilter %f %f %f %f %f %f %f",
      imageFilter.exposure,
      imageFilter.contrast,
      imageFilter.saturation,
      imageFilter.temperature,
      imageFilter.tint,
      imageFilter.highlight,
      imageFilter.shadow);
    return builder.makeColorFilter();
  }
  return nullptr;
}
namespace VGG::layer
{

}; // namespace VGG::layer
