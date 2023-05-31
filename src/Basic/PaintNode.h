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

  Bound2 m_bound;
  glm::mat3 m_transform{ 1.0 };

  std::string guid{};
  std::vector<std::string> maskedBy{};
  Mask outlineMask;
  EMaskType maskType{ MT_None };
  EBoolOp m_clipOperator{ BO_None };

  Style style;
  ContextSetting contextSetting;
  ObjectType type;

  friend class NlohmannBuilder;
  friend class SkiaRenderer;

public:
  PaintNode(const std::string& name, ObjectType type)
    : Node(name)
    , type(type)
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

  glm::mat3 mapTransform(const PaintNode* node) const;

  void setLocalTransform(const glm::mat3& transform)
  {
    this->m_transform = transform;
  }

  const glm::mat3& localTransform() const
  {
    // TODO:: if the node is detached from the parent, this transform should be reset;
    return m_transform;
  }

  const Bound2& getBound() const
  {
    return this->m_bound;
  }

  void setBound(const Bound2& bound)
  {
    this->m_bound = bound;
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

public:
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
  // TODO:: chagne the following functions accessbility
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
    paintPass();
  }

  virtual void postRenderPass(SkCanvas* canvas)
  {
    canvas->restore();
  }

  Mask makeMaskBy(EBoolOp maskOp);

protected:
  virtual void paintPass();
  void renderPass(SkCanvas* canvas); // TODO:: should be private access

  virtual void paintEvent(SkCanvas* canvas)
  {
  }

private:
  void drawDebugBound(SkCanvas* canvas);
};
} // namespace VGG
