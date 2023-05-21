#pragma once
// #include "Basic/Renderer.hpp"
#include "Node.hpp"
#include "VGGType.h"
#include "Geometry.hpp"
#include "VGGUtils.h"
#include "Attrs.h"
#include "RenderState.h"
#include "Scene.hpp"
#include "Mask.h"

#include "glm/matrix.hpp"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/pathops/SkPathOps.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/fwd.hpp"

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
  Mask outlineMask;
  friend class NlohmannBuilder;
  friend class SkiaRenderer;

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

  void addChild(const std::shared_ptr<PaintNode> node)
  {
    pushChildBack(std::move(node));
  }

  void setVisible(bool visible)
  {
    this->visible = visible;
  }

  glm::mat3 mapTransform(const PaintNode* node) const;

  const glm::mat3& localTransform() const
  {
    // TODO:: if the node is detached from the parent, this transform should be reset;
    return transform;
  }

  const Bound2& getBound() const
  {
    return this->bound;
  }

  const std::string& GUID() const
  {
    return guid;
  }

  bool isMasked() const
  {
    return !maskedBy.empty();
  }

  EMaskType getMaskType() const
  {
    return this->maskType;
  }

  /**
   * Return a matrix that transform from this node to the given node
   * */

  virtual SkCanvas* getSkCanvas();
  RenderState* getRenderState();
  void setOutlineMask(const Mask& mask);

  // TODO:: this routine should be removed to a stand alone render pass
  VGG::ObjectTableType PreprocessMask()
  {
    ObjectTableType hash;
    visitNode(this, hash);
    return hash;
  }
  virtual Mask asOutlineMask(const glm::mat3* mat);

  virtual void asAlphaMask();

protected:
  void renderPass(SkCanvas* canvas);

private:
  void visitNode(VGG::Node* p, ObjectTableType& table);

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

public:
  virtual void recursivelyRenderPass(SkCanvas* canvas)
  {
    renderPassBefore();
    for (const auto& p : this->m_firstChild)
    {
      auto q = static_cast<PaintNode*>(p.get());
      q->recursivelyRenderPass(canvas);
    }
    renderPassAfter();
  }
  virtual void renderPassBefore()
  {
    if (isPaintDirty())
    {
      paint();
    }
  }

  virtual void paint();

  virtual void renderPassAfter()
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

  Mask makeMaskBy(EBoolOp maskOp);

private:
  void drawDebugBoarder(SkCanvas* canvas);
  bool isPaintDirty()
  {
    return paintDirty;
  }
};
} // namespace VGG
