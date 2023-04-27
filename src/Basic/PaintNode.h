#pragma once
#include "Node.hpp"
#include "VGGType.h"
#include "Geometry.hpp"
#include "VGGUtils.h"
#include "Attrs.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/fwd.hpp"

#include <any>
#include <unordered_map>
namespace VGG
{

class PaintNode : public Node
{
  static SkCanvas* s_defaultCanvas;
  std::unordered_map<std::string, std::any> properties;

public:
  Bound2 bound;
  glm::mat3 transform;
  ObjectType type;
  bool visible = true;
  bool dirty = false;
  Style style;
  ContextSetting contextSetting;

  PaintNode(const std::string& name, ObjectType type)
    : Node(name)
    , type(type)
  {
  }
  void setVisible(bool visible)
  {
    this->visible = visible;
  }

  void Render(SkCanvas* canvas)
  {
    s_defaultCanvas = canvas;
    traverse();
  }
  virtual SkCanvas* getSkCanvas()
  {
    return s_defaultCanvas;
  }

protected:
  void preVisit() override
  {
    SkCanvas* canvas = getSkCanvas();
    canvas->save();
    canvas->concat(toSkMatrix(this->transform));
    canvas->save();
    canvas->scale(1, -1);
    this->_drawRect(canvas);
    this->Paint(canvas);
    canvas->restore(); // restore the coord convertion
  }

  void postVisit() override
  {
    SkCanvas* canvas = getSkCanvas();
    canvas->restore();
  }

  virtual void Paint(SkCanvas* canvas)
  {
  }

  virtual void Transform(SkCanvas* canvas)
  {
  }

  void propertyChanged()
  {
    markDirty();
  }

  void markDirty()
  {
    this->dirty = true;
  }

  void resetDirty()
  {
    this->dirty = false;
  }

private:
  void _drawRect(SkCanvas* canvas)
  {
    auto skrect = toSkRect(this->bound);
    skrect.setXYWH(skrect.x(), -skrect.y(), skrect.width(), skrect.height());
    SkPaint strokePen;
    strokePen.setStyle(SkPaint::kStroke_Style);
    SkColor color = nodeType2Color(this->type);
    strokePen.setColor(color);
    strokePen.setStrokeWidth(1);
    canvas->drawRect(skrect, strokePen);
  }
};
} // namespace VGG
