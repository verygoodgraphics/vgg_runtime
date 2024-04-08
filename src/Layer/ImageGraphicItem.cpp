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
#include "Effects.hpp"

namespace VGG::layer
{

class ImageBoundsAttribute : public ShapeAttribute
{
public:
  ImageBoundsAttribute(VRefCnt* cnt)
    : ShapeAttribute(cnt, 0)
  {
  }

private:
};

ImageItem::ImageItem(VRefCnt* cnt, ObjectAttribute* object)
  : GraphicItem(cnt)
  , m_objectAttribute(object)
{
  m_imageShape = ShapeAttribute::Make();
  observe(m_imageShape);
}

void ImageItem::render(Renderer* renderer)
{
  DEBUG("render image");
  auto parent = m_objectAttribute.lock();
  if (m_imageShader)
  {
    auto    canvas = renderer->canvas();
    SkPaint p;
    p.setShader(m_imageShader);
    canvas->drawPaint(p);
  }
  ASSERT(m_imageShape);
  if (auto shape = m_imageShape->getShape(); !shape.isEmpty() && parent)
  {
    const auto skBounds = toSkRect(m_imageBounds);
    VGG::layer::drawBorder(renderer, shape, skBounds, parent->getBorderStyle(), 0);
  }
  else
  {
    DEBUG("failed to get image border or shape");
  };
}

Bounds ImageItem::onRevalidate()
{
  if (!m_imageShader)
  {
    m_imageShader = makeStretchPattern(m_imageBounds, m_imagePattern);
    VGG_LAYER_DEBUG_CODE(if (!m_imageShader) {
      DEBUG("failed to create image shader for %s", m_imagePattern.guid.c_str());
    });
  }
  return m_imageBounds;
}

} // namespace VGG::layer
