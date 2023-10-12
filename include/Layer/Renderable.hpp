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
#include "Layer/Config.hpp"

#include <memory>
#include <optional>

class SkCanvas;

namespace VGG::layer
{

struct Viewport
{
  int position[2] = { 0, 0 };
  int extent[2] = { 0, 0 };
};

class VGG_EXPORTS Renderable : public std::enable_shared_from_this<Renderable>
{
  std::optional<Viewport> m_viewport;
  bool m_visible{ true };

protected:
  virtual void onRender(SkCanvas* canvas) = 0;

public:
  void render(SkCanvas* canvas);
  void setViewport(const Viewport& vp)
  {
    m_viewport = vp;
  }

  void setVisible(bool enable)
  {
    m_visible = enable;
  }

  bool visible() const
  {
    return m_visible;
  }
};
} // namespace VGG::layer
