#pragma once
#include "Node.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/fwd.hpp"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "nlohmann/json.hpp"
#include "Geometry.hpp"
#include <memory>
#include <tuple>
#include <unordered_map>
#include <any>
#include <iostream>
#include <utility>

namespace VGG
{

enum ObjectType
{
  VGG_LAYER = 0,
  VGG_PATH,
  VGG_IMAGE,
  VGG_TEXT,
  VGG_ARTBOARD,
  VGG_GROUP,
  VGG_CONTOUR,
  VGG_MASTER,
  VGG_INSTANCE
};

// Node that has transform and bound should be defined as tree node
//
inline SkRect toSkRect(const VGG::Bound2& bound)
{
  auto rect = SkRect::MakeXYWH(bound.bottomLeft.x,
                               bound.bottomLeft.y,
                               bound.topRight.x - bound.bottomLeft.x,
                               bound.topRight.y - bound.bottomLeft.y);
  return rect;
}

inline SkMatrix toSkMatrix(const glm::mat3& mat)
{
  SkMatrix skMatrix;
  skMatrix.setAll(mat[0][0],
                  mat[0][1],
                  mat[0][2],
                  mat[1][0],
                  mat[1][1],
                  mat[1][2],
                  mat[2][0],
                  mat[2][1],
                  mat[2][2]);
  return skMatrix;
}

struct TraversalData
{
  int depth = 0;
  glm::mat3 transform;
};

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
    SkPaint strokePen;
    strokePen.setStyle(SkPaint::kStroke_Style);
    strokePen.setColor(SK_ColorBLUE);
    strokePen.setStrokeWidth(1);
    this->_drawRect(canvas, strokePen);
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
  void _drawRect(SkCanvas* canvas, const SkPaint& paint)
  {
    auto skrect = toSkRect(this->bound);
    canvas->drawRect(skrect, paint);
  }
};

class UnboundNode : public Node
{
};

inline SkCanvas* PaintNode::s_defaultCanvas = nullptr;

class GroupNode final : public PaintNode
{
public:
  GroupNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_GROUP)
  {
  }
};

class ArtboardNode final : public PaintNode
{
public:
  ArtboardNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_ARTBOARD)
  {
  }
};

struct PointAttr
{
  glm::vec2 point;
  float radius;
  int cornerStyle;
  PointAttr(float x, float y, float r, int cornerStyle)
    : point(x, y)
    , radius(r)
    , cornerStyle(cornerStyle)
  {
  }
};

struct Contour : public std::vector<PointAttr>
{
  bool closed = true;
};

class SymbolMasterNode final : public PaintNode
{
public:
  std::string symbolID;
  SymbolMasterNode(const std::string& name)
    : PaintNode(name, VGG_MASTER)
  {
  }
};

class SymbolMasterNode;

class SymbolInstanceNode final : public PaintNode
{
public:
  std::string symbolID;
  std::shared_ptr<SymbolMasterNode> master;
  SymbolInstanceNode(const std::string& name)
    : PaintNode(name, VGG_INSTANCE)
  {
  }

  void Paint(SkCanvas* canvas) override
  {
    if (master == nullptr)
    {
      // lazy init
    }
  }

  void traverse() override
  {
    preVisit();
    if (master)
    {
      master->traverse();
    }
    postVisit();
  }
};

class TextNode final : public PaintNode
{
public:
  TextNode(const std::string& name)
    : PaintNode(name, VGG_TEXT)
  {
  }
};

class ImageNode final : public PaintNode
{
public:
  ImageNode(const std::string& name)
    : PaintNode(name, VGG_IMAGE)
  {
  }
};

class PathNode final : public PaintNode
{
public:
  std::optional<Contour> contour;
  PathNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_PATH)
  {
  }

  void Paint(SkCanvas* canvas) override
  {
  }
};

} // namespace VGG
