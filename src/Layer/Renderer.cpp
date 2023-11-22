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
#include "Renderer.hpp"
#include "Layer/Core/PaintNode.hpp"

#include <core/SkCanvas.h>

namespace
{
inline SkColor nodeType2Color(ObjectType type)
{
  switch (type)
  {
    case ObjectType::VGG_PATH:
      return SK_ColorRED;
    case ObjectType::VGG_IMAGE:
      return SK_ColorRED;
    case ObjectType::VGG_GROUP:
      return SK_ColorRED;
    case ObjectType::VGG_TEXT:
      return SK_ColorRED;
    case ObjectType::VGG_ARTBOARD:
      return SK_ColorRED;
    case ObjectType::VGG_LAYER:
      return SK_ColorRED;
    case ObjectType::VGG_MASTER:
      return SK_ColorRED;
    case ObjectType::VGG_CONTOUR:
      return SK_ColorYELLOW;
    default:
      return SK_ColorRED;
  }
}
} // namespace

namespace VGG::layer
{

void SkiaRenderer::updateMaskObject(PaintNode*                                   p,
                                    std::unordered_map<std::string, PaintNode*>& objects)
{
  if (!p)
    return;
  if (p->maskType() != MT_None)
  {
    if (auto it = objects.find(p->guid()); it == objects.end())
    {
      objects[p->guid()] = p; // type of all children of paintnode must be paintnode
    }
  }
  for (auto it = p->begin(); it != p->end(); ++it)
  {
    updateMaskObject(static_cast<layer::PaintNode*>(it->get()), objects);
  }
}

void SkiaRenderer::drawDebugBound(layer::PaintNode* p, int zorder)
{
  const auto& b = p->bound();
  SkPaint     strokePen;
  strokePen.setStyle(SkPaint::kStroke_Style);
  SkColor color = nodeType2Color(VGG_PATH);
  strokePen.setColor(color);
  // strokePen.setAlpha(zorder2Alpha(zorder));
  strokePen.setStrokeWidth(2);
  m_canvas->drawRect(toSkRect(p->bound()), strokePen);
}
void SkiaRenderer::updateMaskObject(layer::PaintNode* p)
{
  m_maskObjects.clear();
  updateMaskObject(p, m_maskObjects);
}

void SkiaRenderer::draw(SkCanvas* canvas, layer::PaintNode* root)
{
  m_canvas = canvas;
  canvas->save();

  root->renderPass(this);
  canvas->restore();
}

void SkiaRenderer::commit(SkCanvas* canvas)
{
  m_canvas = canvas;
  canvas->save();

  if (!m_enableDrawDebugBound)
  {
    for (const auto& item : m_displayList)
    {
      canvas->save();
      canvas->concat(item.matrix);
      item.item->paintEvent(this);
      canvas->restore();
    }
  }
  else
  {
    for (const auto& item : m_displayList)
    {
      canvas->save();
      canvas->concat(item.matrix);
      drawDebugBound(item.item, item.zorder);
      m_currentMatrix = item.matrix;
      item.item->paintEvent(this);
      canvas->restore();
    }
  }
  canvas->restore();
}

} // namespace VGG::layer
