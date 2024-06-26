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
#include "Pattern.hpp"
#include "Effects.hpp"
#include "Layer/Config.hpp"
#include "LayerCache.h"
#include "VSkia.hpp"
#include <core/SkTileMode.h>

namespace
{
sk_sp<SkShader> createShader(
  const sk_sp<SkImage>&       img,
  SkTileMode                  tileModeX,
  SkTileMode                  tileModeY,
  const SkMatrix&             matrix,
  const sk_sp<SkColorFilter>& colorFilter)
{
  auto shader =
    img->makeShader(tileModeX, tileModeY, VGG::layer::getGlobalSamplingOptions(), matrix);
  if (colorFilter)
  {
    shader = sk_sp<SkShader>(shader->makeWithColorFilter(colorFilter));
  }
  return shader;
}

} // namespace

namespace VGG::layer
{

std::string_view ShaderPattern::init(const std::string& guid)
{
  auto [image, count] = loadImageFromStack(guid, 0);
  if (count > 0 && image)
  {
    m_frames.resize(count);
    m_imageInfo = image->imageInfo();
  }
  else
  {
    return "error"sv;
  }
  m_guid = guid;
  return ""sv;
}

ShaderPattern::ShaderPattern(const Bounds& bounds, const PatternFit& p)
{
  if (auto err = init(p.guid); !err.empty())
  {
    return;
  }

  SkImageInfo mi = m_imageInfo;
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
  m = glm::rotate(m, glm::radians(p.rotation));
  m = glm::translate(m, { -mi.width() / 2, -mi.height() / 2 });
  m_tileModeX = SkTileMode::kDecal;
  m_tileModeY = SkTileMode::kDecal;
  m_matrix = toSkMatrix(m);
  m_colorFilter = makeColorFilter(p.imageFilter);
}
ShaderPattern::ShaderPattern(const Bounds& bounds, const PatternFill& p)
{
  if (auto err = init(p.guid); !err.empty())
  {
    return;
  }

  SkImageInfo mi = m_imageInfo;
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
  m = glm::rotate(m, glm::radians(p.rotation));
  m = glm::translate(m, { -mi.width() / 2, -mi.height() / 2 });
  m_tileModeX = SkTileMode::kDecal;
  m_tileModeY = SkTileMode::kDecal;
  m_colorFilter = makeColorFilter(p.imageFilter);
  m_matrix = toSkMatrix(m);
}
ShaderPattern::ShaderPattern(const Bounds& bounds, const PatternStretch& p)
{
  if (auto err = init(p.guid); !err.empty())
  {
    return;
  }

  SkImageInfo mi = m_imageInfo;
  float       width = bounds.width();
  float       height = bounds.height();
  auto        m = glm::mat3{ 1.0 };
  m = glm::scale(m, { width, height });
  m *= p.transform.matrix();
  m = glm::scale(m, { 1.f / mi.width(), 1.f / mi.height() });
  m_matrix = toSkMatrix(m);
  m_tileModeX = SkTileMode::kDecal;
  m_tileModeY = SkTileMode::kDecal;
  m_colorFilter = makeColorFilter(p.imageFilter);
}
ShaderPattern::ShaderPattern(const Bounds& bounds, const PatternTile& p)
{
  if (auto err = init(p.guid); !err.empty())
  {
    return;
  }

  m_tileModeX = SkTileMode::kDecal;
  m_tileModeY = SkTileMode::kDecal;
  if (p.mode == TILE_VERTICAL)
  {
    m_tileModeY = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  if (p.mode == TILE_HORIZONTAL)
  {
    m_tileModeX = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  if (p.mode == TILE_BOTH)
  {
    m_tileModeY = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
    m_tileModeX = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  auto m = glm::mat3{ 1.0 };
  m = glm::rotate(m, glm::radians(p.rotation));
  m = glm::scale(m, { p.scale, p.scale });
  m_colorFilter = makeColorFilter(p.imageFilter);
  m_matrix = toSkMatrix(m);
}

sk_sp<SkShader> ShaderPattern::shader(int frame) const
{
  ASSERT((int)m_frames.size() == frameCount());
  if (frame < 0 || frame >= frameCount())
  {
    return nullptr;
  }
  if (!m_frames[frame])
  {
    auto [img, total] = loadImageFromStack(m_guid, frame);
    if (!img)
    {
      VGG_LOG_DEV(LOG, Codec, "frame {} is null", frame);
      return nullptr;
    }
    auto shader = createShader(img, m_tileModeX, m_tileModeY, m_matrix, m_colorFilter);
    if (!shader)
    {
      VGG_LOG_DEV(LOG, Codec, "failed to create shader for frame {}", frame);
      return nullptr;
    }
    m_frames[frame] = shader;
  }
  return m_frames[frame];
}
} // namespace VGG::layer
