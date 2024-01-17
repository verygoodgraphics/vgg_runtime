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
#pragma once

#include <cstdint>
#include <unordered_map>

namespace VGG::layer
{

inline const char* g_luminosityBlender = R"(
    const float EPSILON = 1e-10;
    vec3 RGBtoHCV(in vec3 rgb)
    {
        // RGB [0..1] to Hue-Chroma-Value [0..1]
        // Based on work by Sam Hocevar and Emil Persson
        vec4 p = (rgb.g < rgb.b) ? vec4(rgb.bg, -1., 2. / 3.) : vec4(rgb.gb, 0., -1. / 3.);
        vec4 q = (rgb.r < p.x) ? vec4(p.xyw, rgb.r) : vec4(rgb.r, p.yzx);
        float c = q.x - min(q.w, q.y);
        float h = abs((q.w - q.y) / (6. * c + EPSILON) + q.z);
        return vec3(h, c, q.x);
    }
    vec3 RGBtoHSL(in vec3 rgb)
    {
        // RGB [0..1] to Hue-Saturation-Lightness [0..1]
        vec3 hcv = RGBtoHCV(rgb);
        float z = hcv.z - hcv.y * 0.5;
        float s = hcv.y / (1. - abs(z * 2. - 1.) + EPSILON);
        return vec3(hcv.x, s, z);
    }
    vec4 main(vec4 srcColor, vec4 dstColor){ return vec4(dstColor.rgb, RGBtoHSL(srcColor.rgb).z); }
)";

inline const char* g_invLuminosityBlender = R"(
    const float EPSILON = 1e-10;
    vec3 RGBtoHCV(in vec3 rgb)
    {
        // RGB [0..1] to Hue-Chroma-Value [0..1]
        // Based on work by Sam Hocevar and Emil Persson
        vec4 p = (rgb.g < rgb.b) ? vec4(rgb.bg, -1., 2. / 3.) : vec4(rgb.gb, 0., -1. / 3.);
        vec4 q = (rgb.r < p.x) ? vec4(p.xyw, rgb.r) : vec4(rgb.r, p.yzx);
        float c = q.x - min(q.w, q.y);
        float h = abs((q.w - q.y) / (6. * c + EPSILON) + q.z);
        return vec3(h, c, q.x);
    }
    vec3 RGBtoHSL(in vec3 rgb)
    {
        // RGB [0..1] to Hue-Saturation-Lightness [0..1]
        vec3 hcv = RGBtoHCV(rgb);
        float z = hcv.z - hcv.y * 0.5;
        float s = hcv.y / (1. - abs(z * 2. - 1.) + EPSILON);
        return vec3(hcv.x, s, z);
    }
    vec4 main(vec4 srcColor, vec4 dstColor){ return vec4(dstColor.rgb, 1.0 - RGBtoHSL(srcColor.rgb).z); }
)";

inline const char* g_alphaMaskBlender = R"(
    vec4 main(vec4 srcColor, vec4 dstColor){
        return vec4(dstColor.rgb, srcColor.a);
    }
)";

inline const char* g_styleMaskBlenderShader = R"(
    vec4 main(vec4 srcColor, vec4 dstColor){
        vec4 color = srcColor + dstColor * (1.0 - srcColor.a);
        return color * dstColor.a;
    }
)";

inline const char* g_blendModeLinearBurn = R"(
    vec4 main(vec4 srcColor, vec4 dstColor){
        return srcColor.rgba + dstColor.rgba - vec4(1,1,1,1);
    }
)";

} // namespace VGG::layer
