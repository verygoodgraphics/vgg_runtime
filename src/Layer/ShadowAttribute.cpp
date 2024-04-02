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
#include "ShadowAttribute.hpp"
#include "ShapeAttribute.hpp"

#include <core/SkRect.h>

namespace VGG::layer
{

void DropShadowAttribute::render(Renderer* renderer)
{
  if (m_dropShadowEffects && m_shapeAttr)
  {
    if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
    {
      m_dropShadowEffects->render(renderer, shape);
    }
  }
}

Bounds DropShadowAttribute::onRevalidate()
{
  if (m_shapeAttr)
  {
    m_shapeAttr->revalidate();
    auto shape = m_shapeAttr->getShape();
    auto bounds = toSkRect(m_shapeAttr->bound());
    if (!m_shadow.empty() && !shape.isEmpty())
    {
      m_dropShadowEffects = DropShadowEffect(m_shadow, bounds, shape.outset(0, 0).has_value());
      SkRect shadowBounds = m_dropShadowEffects->bounds();
      return Bounds{ shadowBounds.x(),
                     shadowBounds.y(),
                     shadowBounds.width(),
                     shadowBounds.height() };
    }
  }
  return Bounds();
}

Bounds InnerShadowAttribute::onRevalidate()
{
  if (m_shapeAttr)
  {
    m_shapeAttr->revalidate();
    auto shape = m_shapeAttr->getShape();
    if (shape.isEmpty())
      return Bounds();
    auto shapeBounds = m_shapeAttr->getShape().bounds();
    if (!m_shadow.empty() && !shape.isEmpty())
    {
      m_innerShadowEffects = InnerShadowEffect(m_shadow, shapeBounds);
      // SkRect shadowBounds = m_innerShadowEffects->bounds();
      SkRect shadowBounds = shapeBounds;
      return Bounds{ shadowBounds.x(),
                     shadowBounds.y(),
                     shadowBounds.width(),
                     shadowBounds.height() };
    }
  }
  return Bounds();
}

void InnerShadowAttribute::render(Renderer* renderer)
{
  if (m_innerShadowEffects && m_shapeAttr)
  {
    if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
    {
      m_innerShadowEffects->render(renderer, shape);
    }
  }
}
} // namespace VGG::layer
