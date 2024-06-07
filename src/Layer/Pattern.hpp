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

#include "Layer/Core/Attrs.hpp"
#include <core/SkImage.h>
#include <core/SkShader.h>
#include <core/SkMatrix.h>
#include <codec/SkCodec.h>
namespace VGG::layer
{

class ShaderPattern
{
public:
  // Default constructor
  ShaderPattern(const Bounds& bounds, const PatternFit& fit);
  ShaderPattern(const Bounds& bounds, const PatternFill& fill);
  ShaderPattern(const Bounds& bounds, const PatternStretch& strecth);
  ShaderPattern(const Bounds& bounds, const PatternTile& tile);

  ShaderPattern(ShaderPattern&& other) noexcept = default;
  ShaderPattern& operator=(ShaderPattern&& other) noexcept = default;
  ShaderPattern(const ShaderPattern& other) = delete;
  ShaderPattern& operator=(const ShaderPattern& other) = delete;

  bool isValid() const
  {
    return true; // m_codec != nullptr;
  }

  int frameCount() const
  {
    return 1; // m_codec->getFrameCount();
  }

  sk_sp<SkShader> shader(int frame = 0) const;

private:
  std::string_view                     init(const std::string& guid);
  mutable std::vector<sk_sp<SkShader>> m_frames;

  std::unique_ptr<SkCodec> m_codec;
  SkMatrix                 m_matrix;
  sk_sp<SkColorFilter>     m_colorFilter;
  SkTileMode               m_tileModeX, m_tileModeY;

  std::string m_guid;
};

} // namespace VGG::layer
