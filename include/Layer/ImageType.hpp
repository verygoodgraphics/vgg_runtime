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
#pragma once

namespace VGG::layer
{
enum class EImageEncode
{
  IE_PNG,
  IE_JPEG,
  IE_WEBP,
  IE_RAW
};

struct ImageOptions
{
  EImageEncode encode;
  int          position[2] = { 0, 0 };
  int          extend[2] = { 0, 0 };
  int          quality{ 100 };
};
} // namespace VGG::layer
