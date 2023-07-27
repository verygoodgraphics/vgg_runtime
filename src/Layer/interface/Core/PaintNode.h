#pragma once
// #include "Basic/Renderer.hpp"
#include "Common/Config.h"
#include "Core/Node.hpp"
#include "Core/VType.h"
#include "Core/Geometry.hpp"
#include "Core/Attrs.h"
#include "Core/RenderState.h"
#include "Core/Mask.h"
#include "Scene/Scene.h"

#include "glm/matrix.hpp"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/pathops/SkPathOps.h"

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
namespace VGG
{

class PaintNode__pImpl;
class VGG_EXPORTS PaintNode : public Node
{
  VGG_DECL_IMPL(PaintNode)

protected:
  static SkCanvas* s_defaultCanvas;
  static RenderState* s_renderState;
  Bound2 m_bound;
  glm::mat3 m_transform{ 1.0 };

  std::string m_guid{};
  std::vector<std::string> m_maskedBy{};
  Mask m_outlineMask;
  EMaskType m_maskType{ MT_None };
  EBoolOp m_clipOperator{ BO_None };
  EOverflow m_overflow{ OF_Hidden };

  EMaskCoutourType m_maskContourType{ MCT_FrameOnly };

  Style m_style;
  ContextSetting m_contextSetting;
  ObjectType m_type;

  bool m_visible{ true };
  std::optional<Color> m_bgColor;

  friend class NlohmannBuilder;
  friend class SkiaRenderer;

public:
  PaintNode(const std::string& name, ObjectType type);

  void addChild(const std::shared_ptr<PaintNode> node)
  {
    pushChildBack(std::move(node));
  }

  void setContectSettings(const ContextSetting& settings)
  {
    this->m_contextSetting = settings;
  }

  void setOverflow(EOverflow overflow)
  {
    m_overflow = overflow;
  }

  EOverflow overflow() const
  {
    return m_overflow;
  }

  const ContextSetting& contextSetting() const
  {
    return this->m_contextSetting;
  }

  ContextSetting& contextSetting()
  {
    return this->m_contextSetting;
  }

  void setClipOperator(EBoolOp op)
  {
    m_clipOperator = op;
  }

  void setVisible(bool visible)
  {
    this->m_visible = visible;
  }

  void setBackgroundColor(const Color& color)
  {
    this->m_bgColor = color;
  }

  bool isVisible() const
  {
    return this->m_visible;
  }

  void setStyle(const Style& style)
  {
    m_style = style;
  }

  Style& style()
  {
    return m_style;
  }

  const Style& style() const
  {
    return m_style;
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
    return m_guid;
  }

  bool isMasked() const
  {
    return !m_maskedBy.empty();
  }

  EMaskType maskType() const
  {
    return this->m_maskType;
  }

  void setMaskType(EMaskType type)
  {
    this->m_maskType = type;
  }

  EMaskCoutourType maskContourType() const
  {
    return m_maskContourType;
  }

  void setMaskContourType(EMaskCoutourType type)
  {
    m_maskContourType = type;
  }

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
  void invokeRenderPass(SkCanvas* canvas)
  {
    if (!m_visible)
      return;
    preRenderPass(canvas);
    renderOrderPass(canvas);
    postRenderPass(canvas);
  }

  virtual void renderOrderPass(SkCanvas* canvas)
  {

    std::vector<PaintNode*> masked;
    std::vector<PaintNode*> noneMasked;
    for (const auto& p : this->m_firstChild)
    {
      auto c = static_cast<PaintNode*>(p.get());
      if (c->maskType() == MT_Outline)
        masked.push_back(c);
      else
        noneMasked.push_back(c);
    }

    auto paintCall = [&](std::vector<PaintNode*>& nodes)
    {
      if (m_contextSetting.TransparencyKnockoutGroup)
      {
        for (const auto& p : nodes)
        {
          // TODO:: blend mode r = s!=0?s:d is needed.
          // SkPaint paint;
          // paint.setBlendMode(SkBlendMode::kSrc);
          // canvas->save();
          // canvas->scale(1, -1);
          // canvas->saveLayer(toSkRect(getBound()), &paint);
          p->invokeRenderPass(canvas);
          // canvas->restore();
          // canvas->restore();
        }
      }
      else
      {
        for (const auto& p : nodes)
        {
          p->invokeRenderPass(canvas);
        }
      }
    };

    if (overflow() == OF_Hidden)
    {
      canvas->save();
      canvas->clipPath(getContour());
    }
    paintCall(masked);
    paintCall(noneMasked);
    if (overflow() == OF_Hidden)
    {
      canvas->restore();
    }
    // for (const auto& p : this->m_firstChild)
    // {
    //   auto q = static_cast<PaintNode*>(p.get());
    //   q->invokeRenderPass(canvas);
    // }
  }
  virtual void preRenderPass(SkCanvas* canvas)
  {

    if (m_contextSetting.Opacity < 1.0)
    {
      // TODO:: more accurate bound is needed
      canvas->saveLayerAlpha(0, m_contextSetting.Opacity * 255);
    }

    if (m_contextSetting.IsolateBlending)
    {
      // TODO:: blend mode r = s!=0?s:d is needed.
      // SkPaint paint;
      // paint.setBlendMode(SkBlendMode::kSrc);
      // canvas->save();
      // canvas->scale(1, -1);
      // canvas->saveLayer(toSkRect(getBound()), &paint);
    }
    paintPass();
  }

  virtual void postRenderPass(SkCanvas* canvas)
  {
    canvas->restore(); // store the state in paintPass

    if (m_contextSetting.IsolateBlending)
    {
      // canvas->restore();
      // canvas->restore();
    }

    if (m_contextSetting.Opacity < 1.0)
    {
      canvas->restore();
    }
  }

  Mask makeMaskBy(EBoolOp maskOp);

protected:
  virtual SkPath getContour();
  SkPath makeBoundMask();
  SkPath makeOutlineMask(EMaskCoutourType type, const glm::mat3* mat);
  void paintBackgroundColor(SkCanvas* canvas);
  void paintStyle(SkCanvas* canvas);

protected:
  virtual void paintPass();
  void renderPass(SkCanvas* canvas); // TODO:: should be private access

  virtual void paintEvent(SkCanvas* canvas);

private:
  void drawDebugBound(SkCanvas* canvas);
};
} // namespace VGG
