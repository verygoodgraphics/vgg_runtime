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
#include <optional>

#include "VSkia.hpp"

struct LayerContextGuard
{
public:
  std::optional<SkPaint> paint;
  template<typename F>
  void saveLayer(const ContextSetting& st, F&& f)
  {
    auto bm = toSkBlendMode(st.blendMode);
    if (bm)
    {
      std::visit(
        Overloaded{ [&, this](const sk_sp<SkBlender>& blender)
                    {
                      paint = SkPaint();
                      paint->setBlender(blender);
                    },
                    [&, this](const SkBlendMode& mode)
                    {
                      paint = SkPaint();
                      paint->setBlendMode(mode);
                    } },
        *bm);
    }

    if (st.opacity < 1.0)
    {
      if (!paint)
        paint = SkPaint();
      paint->setAlphaf(st.opacity);
    }
    if (paint)
    {
      f(*paint);
    }
  }
  template<typename F>
  void restore(F&& f)
  {
    if (paint)
    {
      f();
    }
    paint = std::nullopt;
  }
};
