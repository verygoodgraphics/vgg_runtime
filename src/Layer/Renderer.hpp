#pragma once
#include "VSkia.hpp"
#include "PaintNodePrivate.hpp"

#include "Layer/Core/PaintNode.hpp"
#include "Layer/Config.hpp"
#include "RenderState.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/matrix.hpp"

#include <core/SkMatrix.h>
#include <include/core/SkCanvas.h>

namespace VGG
{
// NOLINTBEGIN
inline void PrintSkMatrix(const SkMatrix& m)
{
  for (int i = 0; i < 9; i++)
  {
    std::cout << m[i] << " ";
  }
  std::cout << std::endl;
}
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
  PaintNode* item{ nullptr };

  DisplayItem(SkMatrix m, PaintNode* item)
    : matrix(std::move(m))
    , item(item)
  {
  }
};

inline std::ostream& operator<<(std::ostream& os, const glm::mat3 m)
{
  os << "[" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << std::endl;
  os << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << std::endl;
  os << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << "]\n";
  return os;
}

// NOLINTEND
class SkiaRenderer
{
  RenderState m_state;
  SkCanvas* m_canvas{ nullptr };
  bool m_enableDrawDebugBound{ false };
  std::unordered_map<std::string, PaintNode*> m_maskObjects;
  Scene* m_scene{ nullptr };
  std::vector<glm::mat3> m_transforms;
  std::vector<DisplayItem> m_displayList;

  void updateMaskObject(PaintNode* p, std::unordered_map<std::string, PaintNode*>& objects)
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
      updateMaskObject(static_cast<PaintNode*>(it->get()), objects);
    }
  }

public:
  void drawDebugBound(PaintNode* p)
  {
    const auto& b = p->getBound();
    SkPaint strokePen;
    strokePen.setStyle(SkPaint::kStroke_Style);
    SkColor color = nodeType2Color(p->d_ptr->type);
    strokePen.setColor(color);
    strokePen.setStrokeWidth(2);
    m_canvas->drawRect(toSkRect(p->getBound()), strokePen);
  }
  void updateMaskObject(PaintNode* p)
  {
    m_maskObjects.clear();
    updateMaskObject(p, m_maskObjects);
  }
  const std::unordered_map<std::string, PaintNode*>& maskObjects() const
  {
    return m_maskObjects;
  }

  void draw(SkCanvas* canvas, PaintNode* root)
  {
    m_canvas = canvas;
    canvas->save();
    canvas->scale(1, -1); // convert the whole root to canvas coords
    root->renderPass(this);
    canvas->restore();
  }

  void pushMatrix(glm::mat3 matrix)
  {
    m_transforms.push_back(matrix);
  }

  void pushItem(PaintNode* item)
  {
    auto m = glm::identity<glm::mat3>();
    for (const auto& ma : m_transforms)
    {
      m = m * ma;
    }
    m_displayList.emplace_back(toSkMatrix(m), item);
  }

  void popMatrix()
  {
    m_transforms.pop_back();
  }

  void clearCache()
  {
    m_displayList.clear();
    m_transforms.clear();
  }

  void enableDrawDebugBound(bool enabled)
  {
    m_enableDrawDebugBound = enabled;
  }
  bool isEnableDrawDebugBound()
  {
    return m_enableDrawDebugBound;
  }

  SkCanvas* canvas()
  {
    return m_canvas;
  }

  void commit(SkCanvas* canvas)
  {
    m_canvas = canvas;
    canvas->save();
    canvas->scale(1, -1); // convert the whole root to canvas coords

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
        drawDebugBound(item.item);
        item.item->paintEvent(this);
        canvas->restore();
      }
    }
    canvas->restore();
  }
};

} // namespace VGG
