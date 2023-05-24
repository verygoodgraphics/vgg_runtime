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
  std::vector<std::string> maskedBy;
  Mask outlineMask;
  EMaskType maskType{ MT_None };
  bool paintDirty{ false };
  EBoolOp m_clipOperator;

  friend class NlohmannBuilder;
  friend class SkiaRenderer;

public:
  Bound2 bound;
  glm::mat3 transform{ 1.0 };
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

  void setClipOperator(EBoolOp op)
  {
    m_clipOperator = op;
  }

  EBoolOp clipOperator() const
  {
    return m_clipOperator;
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
  void renderPass(SkCanvas* canvas); // TODO:: should be private access

  virtual void paintEvent(SkCanvas* canvas)
  {
  }

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
  void invokeRenderPass(SkCanvas* canvas)
  {
    preRenderPass(canvas);
    renderOrderPass(canvas);
    postRenderPass(canvas);
  }

  virtual void renderOrderPass(SkCanvas* canvas)
  {
    for (const auto& p : this->m_firstChild)
    {
      auto q = static_cast<PaintNode*>(p.get());
      q->invokeRenderPass(canvas);
    }
  }
  virtual void preRenderPass(SkCanvas* canvas)
  {
    if (isPaintDirty())
    {
      paintPass();
    }
  }

  virtual void paintPass();

  virtual void postRenderPass(SkCanvas* canvas)
  {
    canvas->restore();
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
