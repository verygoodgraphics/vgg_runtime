#pragma once
#include "Node.hpp"
#include "VGGType.h"
#include "Geometry.hpp"
#include "VGGUtils.h"
#include "Attrs.h"
#include "RenderState.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/fwd.hpp"
#include "include/core/SkPath.h"

#include <any>
#include <memory>
#include <string>
#include <unordered_map>
namespace VGG
{

class PaintNode : public Node
{
protected:
  static SkCanvas* s_defaultCanvas;
  static RenderState* s_renderState;
  std::string guid;
  std::unordered_map<std::string, std::any> properties;
  bool paintDirty{ false };
  EMaskType maskType{ MT_None };
  std::vector<std::string> maskedBy;
  SkPath outlineMask;
  friend class NlohmannBuilder;

public:
  Bound2 bound;
  glm::mat3 transform;
  ObjectType type;
  bool visible = true;
  Style style;
  ContextSetting contextSetting;

  PaintNode(const std::string& name, ObjectType type)
    : Node(name)
    , type(type)
    , paintDirty(true)
  {
  }
  void setVisible(bool visible)
  {
    this->visible = visible;
  }

  const std::string& GUID()
  {
    return guid;
  }

  void Render(SkCanvas* canvas)
  {
    s_defaultCanvas = canvas;
    RenderState renderState;
    s_renderState = &renderState;
    traverse();
    s_renderState = nullptr;
  }
  virtual SkCanvas* getSkCanvas()
  {
    return s_defaultCanvas;
  }

  RenderState* getRenderState()
  {
    return s_renderState;
  }

  void setOutlineMask(const SkPath& path)
  {
    outlineMask = path;
  }

  // TODO:: this routine should be removed to a stand alone render pass
  void PreprocessMask()
  {
    std::unordered_map<std::string, std::weak_ptr<PaintNode>> hash;
    visitNode(this, hash);
    visitFunc(this,
              [&hash](VGG::PaintNode* node)
              {
                if (node->maskedBy.empty() == false)
                {
                  SkPath outlinePath;
                  for (const auto& maskID : node->maskedBy)
                  {
                    if (auto it = hash.find(maskID); it != hash.end())
                    {
                      if (it->second.lock()->type == VGG_PATH) // only handle path mask so far
                      {
                        // TODO::
                      }
                    }
                  }
                  node->setOutlineMask(outlinePath);
                }
              });
  }

private:
  void visitNode(VGG::Node* p, std::unordered_map<std::string, std::weak_ptr<PaintNode>>& table)
  {
    if (!p)
      return;
    if (auto it = table.find(p->getName()); it == table.end())
    {
      auto sptr = std::static_pointer_cast<PaintNode>(p->shared_from_this());
      table[sptr->GUID()] = sptr; // type of all children of paintnode must be paintnode
    }
    for (const auto& c : m_firstChild)
    {
      visitNode(c.get(), table);
    }
  }

  template<typename F>
  void visitFunc(VGG::Node* p, F&& f)
  {
    if (!p)
      return;
    f(static_cast<PaintNode*>(p));
    for (const auto& c : m_firstChild)
    {
      visitFunc(c.get(), std::forward<F>(f));
    }
  }

protected:
  void preVisit() override
  {
    if (isPaintDirty())
    {
      if (maskType != MT_None)
      {
      }
      paint();
      // std::cout << "paint\n";
      // this->resetPaintDirty();
    }
  }

  virtual void paint()
  {
    SkCanvas* canvas = getSkCanvas();
    canvas->save();
    canvas->concat(toSkMatrix(this->transform));
    canvas->save();
    canvas->scale(1, -1);
    this->drawDebugBoarder(canvas);
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

  void markPaintDirty()
  {
    this->paintDirty = true;
  }

  void resetPaintDirty()
  {
    this->paintDirty = false;
  }

private:
  void drawDebugBoarder(SkCanvas* canvas)
  {
    auto skrect = toSkRect(this->bound);
    SkPaint strokePen;
    strokePen.setStyle(SkPaint::kStroke_Style);
    SkColor color = nodeType2Color(this->type);
    strokePen.setColor(color);
    strokePen.setStrokeWidth(1);
    canvas->drawRect(skrect, strokePen);
  }
  bool isPaintDirty()
  {
    return paintDirty;
  }
};
} // namespace VGG
