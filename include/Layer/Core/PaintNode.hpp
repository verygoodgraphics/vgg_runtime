#pragma once

#include "Math/Geometry.hpp"
#include "Layer/Core/Node.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Mask.hpp"
#include "Layer/Config.hpp"
#include "Layer/Scene.hpp"

#include <glm/matrix.hpp>
// #include <include/core/SkColor.h>
// #include <include/core/SkMatrix.h>
// #include <include/core/SkPaint.h>
// #include <include/pathops/SkPathOps.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
class SkCanvas;
namespace VGG
{

class RenderState;
class SkiaRenderer;

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

// NOLINTBEGIN
enum EPaintStrategy
{
  PS_Recursively,
  PS_SelfOnly,
  PS_ChildOnly,
};
// NOLINTEND

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

  virtual Mask asOutlineMask(const glm::mat3* mat);
  virtual void asAlphaMask();

  ~PaintNode();

public:
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
  void invokeRenderPass(SkiaRenderer* renderer);

protected:
  // Render traverse
  virtual void paintChildrenPass(SkiaRenderer* renderer);
  void paintChildrenRecursively(SkiaRenderer* renderer);
  virtual void prePaintPass(SkiaRenderer* renderer);
  virtual void postPaintPass(SkiaRenderer* renderer);
  virtual void paintPass(SkiaRenderer* renderer);
  void renderPass(SkiaRenderer* renderer); // TODO:: should be private access
  virtual void paintEvent(SkiaRenderer* renderer);

protected:
  // Mask
  SkPath makeBoundPath();
  virtual SkPath makeContourImpl(ContourOption option, const glm::mat3* mat);
  SkPath childPolyOperation() const;
  Mask makeMaskBy(EBoolOp maskOp, SkiaRenderer* renderer);

protected:
protected:
  // Style
  virtual SkPath stylePath();
  void paintStyle(SkCanvas* canvas, const SkPath& path, const SkPath& mask);
  virtual void paintFill(SkCanvas* canvas,
                         const SkPath& path); // TODO:: only for ImageNode overriding
};
} // namespace VGG
