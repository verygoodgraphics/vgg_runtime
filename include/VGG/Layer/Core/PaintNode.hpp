#pragma once

#include "VGG/Math/Geometry.hpp"
#include "VGG/Layer/Core/Node.hpp"
#include "VGG/Layer/Core/VType.hpp"
#include "VGG/Layer/Core/Attrs.hpp"
#include "VGG/Layer/Core/Mask.hpp"
#include "VGG/Layer/Config.hpp"
#include "VGG/Layer/Scene.hpp"

#include <glm/matrix.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkColor.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <include/pathops/SkPathOps.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
namespace VGG
{

class RenderState;

struct ContourOption
{
  ECoutourType contourType{ ECoutourType::MCT_FrameOnly };
  bool visibilityAware{ true };
  ContourOption(ECoutourType contourType = ECoutourType::MCT_FrameOnly,
                bool visibilityAware = false)
    : contourType(contourType)
    , visibilityAware(visibilityAware)
  {
  }
};

enum EPaintStrategy
{
  PS_Recursively,
  PS_SelfOnly,
  PS_ChildOnly,
};

struct PaintOption
{
  EPaintStrategy paintStrategy{ PS_Recursively };
  PaintOption(EPaintStrategy paintStrategy = PS_Recursively)
    : paintStrategy(paintStrategy)
  {
  }
};

class PaintNode__pImpl;
class VGG_EXPORTS PaintNode : public Node
{
protected:
  VGG_DECL_IMPL(PaintNode)

protected:
  static SkCanvas* s_defaultCanvas;
  static RenderState* s_renderState;
  friend class NlohmannBuilder;
  friend class SkiaRenderer;

private:
  PaintNode(const std::string& name, std::unique_ptr<PaintNode__pImpl> impl);

public:
  PaintNode(const std::string& name, ObjectType type, const std::string& guid);
  PaintNode(const PaintNode&);
  PaintNode& operator=(const PaintNode&) = delete;
  PaintNode(PaintNode&&) = default;
  PaintNode& operator=(PaintNode&&) = default;

  virtual NodePtr clone() const override;

  void addChild(const std::shared_ptr<PaintNode> node)
  {
    if (!node)
      return;
    pushChildBack(std::move(node));
  }

  void addSubShape(const std::shared_ptr<PaintNode> node, EBoolOp op)
  {
    if (!node)
      return;
    node->setClipOperator(op);
    addChild(node);
  }

  void setContectSettings(const ContextSetting& settings);

  void setOverflow(EOverflow overflow);

  EOverflow overflow() const;

  const ContextSetting& contextSetting() const;

  ContextSetting& contextSetting();

  void setClipOperator(EBoolOp op);

  void setVisible(bool visible);

  void setChildWindingType(EWindingType rule);

  EWindingType childWindingType() const;

  bool isVisible() const;

  void setStyle(const Style& style);

  Style& style();

  const Style& style() const;

  EBoolOp clipOperator() const;

  glm::mat3 mapTransform(const PaintNode* node) const;

  void setLocalTransform(const glm::mat3& transform);

  const glm::mat3& localTransform() const;

  const Bound2& getBound() const;

  void setBound(const Bound2& bound);

  const std::string& guid() const;

  bool isMasked() const;

  void setMaskBy(const std::vector<std::string> masks);

  EMaskType maskType() const;

  void setMaskType(EMaskType type);

  void setContourOption(ContourOption option);

  void setContourData(ContourPtr contour);

  Contour* contour();

  const ContourOption& maskOption() const;

  void setPaintOption(PaintOption option);

  const PaintOption& paintOption() const;

  /**
   * Return a matrix that transform from this node to the given node
   * */
  virtual SkCanvas* getSkCanvas();
  RenderState* getRenderState();
  void setOutlineMask(const Mask& mask);

  // TODO:: this routine should be removed to a stand alone render pass
  VGG::ObjectTableType preprocessMask()
  {
    ObjectTableType hash;
    visitNode(this, hash);
    return hash;
  }

  virtual Mask asOutlineMask(const glm::mat3* mat);
  virtual void asAlphaMask();

  ~PaintNode();

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
  void invokeRenderPass(SkCanvas* canvas);

protected:
  // Render traverse
  virtual void paintChildrenPass(SkCanvas* canvas);
  void paintChildrenRecursively(SkCanvas* canvas);
  virtual void prePaintPass(SkCanvas* canvas);
  virtual void postPaintPass(SkCanvas* canvas);
  virtual void paintPass();
  void renderPass(SkCanvas* canvas); // TODO:: should be private access
  virtual void paintEvent(SkCanvas* canvas);

protected:
  // Mask
  SkPath makeBoundPath();
  virtual SkPath makeContourImpl(ContourOption option, const glm::mat3* mat);
  SkPath childPolyOperation() const;
  Mask makeMaskBy(EBoolOp maskOp);

protected:
protected:
  // Style
  virtual SkPath stylePath();
  void paintStyle(SkCanvas* canvas, const SkPath& path);
  virtual void paintFill(SkCanvas* canvas,
                         const SkPath& path); // TODO:: only for ImageNode overriding

private:
  void drawDebugBound(SkCanvas* canvas);
};
} // namespace VGG
