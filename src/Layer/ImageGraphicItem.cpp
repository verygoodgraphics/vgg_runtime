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

#include "ImageGraphicItem.hpp"
#include "ObjectAttribute.hpp"
#include "ShapeAttribute.hpp"
#include "Layer/StyleItem.hpp"
#include "Effects.hpp"

namespace VGG::layer
{

ImageItem::ImageItem(VRefCnt* cnt, StyleItem* object)
  : GraphicItem(cnt)
{
  m_imageShape = ShapeAttributeImpl::Make();
  m_brush = Brush::Make();
  Pattern patt;
  patt.instance = PatternStretch{};
  m_brush->setBrush(std::move(patt));
  observe(m_imageShape);
  observe(m_brush);
}

void ImageItem::render(Renderer* renderer)
{
  DEBUG("render image");
  auto canvas = renderer->canvas();
  if (auto p = m_brush->paint(getImageBounds()); p.getShader())
  {
    canvas->drawPaint(p);
  }
}

Bounds ImageItem::onRevalidate(Revalidation* inv, const glm::mat3& mat)
{
  m_brush->revalidate();
  return m_imageBounds;
}

} // namespace VGG::layer
