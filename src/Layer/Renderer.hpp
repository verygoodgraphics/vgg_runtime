#pragma once

#include "VGG/Layer/Core/PaintNode.hpp"
#include "VGG/Layer/Config.hpp"
#include "RenderState.hpp"

#include <include/core/SkCanvas.h>

namespace VGG
{
// NOLINTBEGIN
// inline void PrintSkMatrix(const SkMatrix& m)
// {
//   for (int i = 0; i < 9; i++)
//   {
//     std::cout << m[i] << " ";
//   }
//   std::cout << std::endl;
// }
//
// inline void PrintCurrentTranslation(SkCanvas* canvas, int indent)
// {
//   auto m = canvas->getTotalMatrix();
//   SkVector v;
//   m.mapVector(0, 0, &v);
//   for (int i = 0; i < indent; i++)
//   {
//     std::cout << "\t";
//   }
//   std::cout << v.x() << " " << v.y() << std::endl;
//   std::cout << std::endl;
// }

struct DisplayItem
{
  SkMatrix matrix;
  PaintNode* item;
  PaintNode* mask;
};

// NOLINTEND
class SkiaRenderer
{
  RenderState m_state;
  std::vector<DisplayItem> m_displayList;
  SkCanvas* m_canvas{ nullptr };

public:
  void draw(SkCanvas* canvas, PaintNode* root)
  {
    m_canvas = canvas;
    canvas->save();
    canvas->scale(1, -1); // convert the whole root to canvas coords
    root->renderPass(this);
    canvas->restore();
  }

  SkCanvas* canvas()
  {
    return m_canvas;
  }

  void draw(SkCanvas* canvas)
  {
    m_canvas = canvas;
    SkMatrix savedMatrix = canvas->getTotalMatrix();
    for (const auto& item : m_displayList)
    {
      canvas->setMatrix(item.matrix);
      item.item->paintEvent(this);
    }
    canvas->setMatrix(savedMatrix);
  }
};

} // namespace VGG
