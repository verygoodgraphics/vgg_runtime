#include "Core/PaintNode.h"
#include "Core/Node.h"
#include "Core/VType.h"
#include "SkiaImpl/VSkia.h"
#include "core/SkCanvas.h"
#include "Core/StyleRenderer.h"
#include <core/SkPath.h>
#include <core/SkRRect.h>
#include <limits>

namespace VGG
{

SkCanvas* PaintNode::s_defaultCanvas = nullptr;
RenderState* PaintNode::s_renderState = nullptr;

class PaintNode__pImpl
{
  VGG_DECL_API(PaintNode);

public:
  Bound2 bound;
  glm::mat3 transform{ 1.0 };
  std::string guid{};
  std::vector<std::string> maskedBy{};
  Mask outlineMask;
  EMaskType maskType{ MT_None };
  EBoolOp clipOperator{ BO_None };
  EOverflow overflow{ OF_Hidden };
  EMaskCoutourType maskContourType{ MCT_FrameOnly };
  Style style;
  ContextSetting contextSetting;
  ObjectType type;
  bool visible{ true };
  std::optional<Color> bgColor;
  PaintNode__pImpl(PaintNode* api, ObjectType type)
    : q_ptr(api)
    , type(type)
  {
  }
};

PaintNode::PaintNode(const std::string& name, ObjectType type, const std::string& guid)
  : Node(name)
  , d_ptr(new PaintNode__pImpl(this, type))
{
  d_ptr->guid = guid;
}

void PaintNode::setContectSettings(const ContextSetting& settings)
{
  VGG_IMPL(PaintNode);
  _->contextSetting = settings;
}

glm::mat3 PaintNode::mapTransform(const PaintNode* node) const
{
  auto findPath = [](const Node* node) -> std::vector<const Node*>
  {
    std::vector<const Node*> path = { node };
    while (node->parent())
    {
      node = node->parent().get();
      path.push_back(node);
    }
    return path;
  };
  auto path1 = findPath(node);
  auto path2 = findPath(this);
  const Node* lca = nullptr;
  int lcaIdx = -1;
  for (int i = path1.size() - 1, j = path2.size() - 1; i >= 0 && j >= 0; i--, j--)
  {
    auto n1 = path1[i];
    auto n2 = path2[j];
    if (n1 == n2)
    {
      lca = n1;
      lcaIdx = j;
    }
    else
    {
      break;
    }
  }
  glm::mat3 mat{ 1.0 };
  if (!lca)
    return mat;
  for (int i = 0; i < path1.size() && path1[i] != lca; i++)
  {
    auto skm = static_cast<const PaintNode*>(path1[i])->d_ptr->transform;
    auto inv = glm::inverse(skm);
    mat = mat * inv;
  }

  for (int i = lcaIdx - 1; i >= 0; i--)
  {
    const auto m = static_cast<const PaintNode*>(path2[i])->d_ptr->transform;
    mat = mat * m;
  }
  return mat;
}

Mask PaintNode::makeMaskBy(EBoolOp maskOp)
{
  VGG_IMPL(PaintNode);
  Mask result;
  if (_->maskedBy.empty())
    return result;

  auto op = toSkPathOp(maskOp);
  auto objects = Scene::getObjectTable();
  for (const auto id : _->maskedBy)
  {
    if (id != this->guid())
    {
      auto obj = objects[id].lock().get();
      const auto t = obj->mapTransform(this);
      auto m = obj->asOutlineMask(&t);
      if (result.outlineMask.isEmpty())
      {
        result = m;
      }
      else
      {
        Op(result.outlineMask, m.outlineMask, op, &result.outlineMask);
      }
    }
  }
  return result;
}

void PaintNode::renderPass(SkCanvas* canvas)
{
  s_defaultCanvas = canvas;
  RenderState renderState;
  s_renderState = &renderState;
  invokeRenderPass(canvas);
  s_renderState = nullptr;
}

void PaintNode::drawDebugBound(SkCanvas* canvas)
{
  VGG_IMPL(PaintNode);
  const auto& b = getBound();
  SkPaint strokePen;
  strokePen.setStyle(SkPaint::kStroke_Style);
  SkColor color = nodeType2Color(_->type);
  strokePen.setColor(color);
  strokePen.setStrokeWidth(2);
  canvas->drawRect(toSkRect(getBound()), strokePen);
}
void PaintNode::visitNode(VGG::Node* p, ObjectTableType& table)
{
  if (!p)
    return;
  auto sptr = std::static_pointer_cast<PaintNode>(p->shared_from_this());
  if (sptr->d_ptr->maskType != MT_None)
  {
    if (auto it = table.find(sptr->guid()); it == table.end())
    {
      table[sptr->guid()] = sptr; // type of all children of paintnode must be paintnode
    }
  }
  for (auto it = p->begin(); it != p->end(); ++it)
  {
    visitNode(it->get(), table);
  }
}

void PaintNode::paintPass()
{
  VGG_IMPL(PaintNode);
  SkCanvas* canvas = getSkCanvas();
  canvas->save();
  canvas->concat(toSkMatrix(_->transform));
  if (Scene::isEnableDrawDebugBound())
  {
    this->drawDebugBound(canvas);
  }
  this->paintEvent(canvas);
}

SkCanvas* PaintNode::getSkCanvas()
{
  return s_defaultCanvas;
}

RenderState* PaintNode::getRenderState()
{
  return s_renderState;
}

void PaintNode::paintEvent(SkCanvas* canvas)
{
  const auto path = stylePath();
  paintBackgroundColor(canvas, path); // TODO::remove
  paintStyle(canvas, path);
}

void PaintNode::paintBackgroundColor(SkCanvas* canvas, const SkPath& path)
{

  VGG_IMPL(PaintNode);
  if (_->bgColor.has_value())
  {
    SkPaint bgPaint;
    bgPaint.setColor(_->bgColor.value());
    bgPaint.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path, bgPaint);
  }
}

void PaintNode::setMaskBy(std::vector<std::string> masks)
{
  VGG_IMPL(PaintNode);
  _->maskedBy = std::move(masks);
}

SkPath PaintNode::makeBoundMask()
{
  SkPath p;
  const auto& skRect = toSkRect(getBound());
  const auto radius = style().frameRadius;

  bool rounded = false;
  float maxR = 0.0, minR = std::numeric_limits<float>::max();
  for (const auto r : radius)
  {
    if (r > 0.f)
    {
      rounded = true;
    }
    maxR = std::max(maxR, r);
    minR = std::min(minR, r);
  }
  if (rounded && (maxR - minR) < std::numeric_limits<float>::epsilon())
  {
    p.addRoundRect(skRect, minR, minR);
  }
  else if (rounded)
  {
    // TODO:: create general path
    p.addRoundRect(skRect, minR, minR);
  }
  else
  {
    p.addRect(skRect);
  }
  return p;
}

SkPath PaintNode::makeOutlineMask(EMaskCoutourType type, const glm::mat3* mat)
{
  SkPath path;
  if (!hasChild())
  {
    SkPath path;
    if (mat)
    {
      path.transform(toSkMatrix(*mat));
    }
    return path;
  }

  switch (type)
  {
    case MCT_FrameOnly:
      this->makeBoundMask();
      break;
    case MCT_UnionWithFrame:
      path = this->makeBoundMask();
    case MCT_Union:
    {
      for (const auto& c : m_firstChild)
      {
        auto paintNode = static_cast<PaintNode*>(c.get());
        auto childMask = paintNode->asOutlineMask(&paintNode->localTransform());
        Op(path, childMask.outlineMask, SkPathOp::kUnion_SkPathOp, &path);
      }
    }
    break;
    case MCT_IntersectWithFrame:
      path = this->makeBoundMask();
    case MCT_Intersect:
    {
      for (const auto& c : m_firstChild)
      {
        auto paintNode = static_cast<PaintNode*>(c.get());
        auto childMask = paintNode->asOutlineMask(&paintNode->localTransform());
        Op(path, childMask.outlineMask, SkPathOp::kIntersect_SkPathOp, &path);
      }
    }
    break;
    case MCT_UnionDependsOn:
    {
      for (const auto& c : m_firstChild)
      {
        auto paintNode = static_cast<PaintNode*>(c.get());
        auto childMask = paintNode->asOutlineMask(&paintNode->localTransform());
        Op(path, childMask.outlineMask, SkPathOp::kUnion_SkPathOp, &path);
      }
    }
    break;
    case MCT_IntersectDependsOn:
    {
      for (const auto& c : m_firstChild)
      {
        auto paintNode = static_cast<PaintNode*>(c.get());
        auto childMask =
          paintNode->makeOutlineMask(paintNode->maskContourType(), &paintNode->localTransform());
        Op(path, childMask, SkPathOp::kUnion_SkPathOp, &path);
      }
    }
    break;
  }
  if (mat)
  {
    path.transform(toSkMatrix(*mat));
  }
  return path;
}

Mask PaintNode::asOutlineMask(const glm::mat3* mat)
{
  Mask mask;
  mask.outlineMask = makeOutlineMask(maskContourType(), mat);
  return mask;
}

void PaintNode::setOutlineMask(const Mask& mask)
{
  VGG_IMPL(PaintNode);
  _->outlineMask = mask;
}

void PaintNode::asAlphaMask()
{
}

void PaintNode::setOverflow(EOverflow overflow)
{

  VGG_IMPL(PaintNode);
  _->overflow = overflow;
}

EOverflow PaintNode::overflow() const
{
  return d_ptr->overflow;
}

const ContextSetting& PaintNode::contextSetting() const
{
  return d_ptr->contextSetting;
}

ContextSetting& PaintNode::contextSetting()
{
  VGG_IMPL(PaintNode);
  return _->contextSetting;
}

void PaintNode::setClipOperator(EBoolOp op)
{
  VGG_IMPL(PaintNode);
  _->clipOperator = op;
}

void PaintNode::setVisible(bool visible)
{
  VGG_IMPL(PaintNode);
  _->visible = visible;
}

void PaintNode::setBackgroundColor(const Color& color)
{
  VGG_IMPL(PaintNode);
  _->bgColor = color;
}

bool PaintNode::isVisible() const
{
  return d_ptr->visible;
}

void PaintNode::setStyle(const Style& style)
{
  VGG_IMPL(PaintNode);
  _->style = style;
}

Style& PaintNode::style()
{
  VGG_IMPL(PaintNode);
  return _->style;
}

const Style& PaintNode::style() const
{
  return d_ptr->style;
}

EBoolOp PaintNode::clipOperator() const
{
  return d_ptr->clipOperator;
}
void PaintNode::setLocalTransform(const glm::mat3& transform)
{
  VGG_IMPL(PaintNode);
  _->transform = transform;
}

const glm::mat3& PaintNode::localTransform() const
{
  // TODO:: if the node is detached from the parent, this transform should be reset;
  return d_ptr->transform;
}

const Bound2& PaintNode::getBound() const
{
  return d_ptr->bound;
}

void PaintNode::setBound(const Bound2& bound)
{
  VGG_IMPL(PaintNode);
  _->bound = bound;
}

const std::string& PaintNode::guid() const
{
  return d_ptr->guid;
}

bool PaintNode::isMasked() const
{
  return d_ptr->maskedBy.empty();
}

EMaskType PaintNode::maskType() const
{
  return d_ptr->maskType;
}

void PaintNode::setMaskType(EMaskType type)
{
  VGG_IMPL(PaintNode);
  _->maskType = type;
}

EMaskCoutourType PaintNode::maskContourType() const
{
  return d_ptr->maskContourType;
}

void PaintNode::setMaskContourType(EMaskCoutourType type)
{
  VGG_IMPL(PaintNode);
  d_ptr->maskContourType = type;
}

void PaintNode::invokeRenderPass(SkCanvas* canvas)
{
  VGG_IMPL(PaintNode);
  if (!_->visible)
    return;
  prePaintPass(canvas);
  paintChildrenPass(canvas);
  postPaintPass(canvas);
}

void PaintNode::paintChildrenPass(SkCanvas* canvas)
{
  VGG_IMPL(PaintNode);
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
    if (_->contextSetting.TransparencyKnockoutGroup)
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
    canvas->clipPath(makeBoundMask());
  }
  paintCall(masked);
  paintCall(noneMasked);
  if (overflow() == OF_Hidden)
  {
    canvas->restore();
  }
}
void PaintNode::prePaintPass(SkCanvas* canvas)
{
  VGG_IMPL(PaintNode);
  if (_->contextSetting.Opacity < 1.0)
  {
    // TODO:: more accurate bound is needed
    canvas->saveLayerAlpha(0, _->contextSetting.Opacity * 255);
  }

  if (_->contextSetting.IsolateBlending)
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

void PaintNode::postPaintPass(SkCanvas* canvas)
{
  VGG_IMPL(PaintNode);
  canvas->restore(); // store the state in paintPass

  if (_->contextSetting.IsolateBlending)
  {
    // canvas->restore();
    // canvas->restore();
  }

  if (_->contextSetting.Opacity < 1.0)
  {
    canvas->restore();
  }
}

SkPath PaintNode::stylePath()
{
  return makeBoundMask();
}

void PaintNode::paintStyle(SkCanvas* canvas, const SkPath& path)
{
  StyleRenderer styleRenderer;
  // draw blur, we assume that there is only one blur style
  bool hasBlur = style().blurs.empty() ? false : style().blurs[0].isEnabled;
  if (hasBlur)
  {
    auto pen = styleRenderer.makeBlurPen(style().blurs[0]);
    canvas->saveLayer(nullptr, &pen);
  }

  auto mask = makeMaskBy(BO_Intersection);
  if (mask.outlineMask.isEmpty())
  {
    styleRenderer.drawContour(canvas,
                              contextSetting(),
                              style(),
                              path,
                              getBound(),
                              [&]() { paintFill(canvas, path); });
  }
  else
  {
    canvas->save();
    canvas->clipPath(mask.outlineMask);
    styleRenderer.drawContour(canvas,
                              contextSetting(),
                              style(),
                              path,
                              getBound(),
                              [&]() { paintFill(canvas, path); });
    canvas->restore();
  }

  // restore blur
  if (hasBlur)
  {
    canvas->restore();
  }
}

void PaintNode::paintFill(SkCanvas* canvas, const SkPath& path)
{
  StyleRenderer render;
  render.drawFill(canvas, contextSetting().Opacity, style(), path, getBound());
}

PaintNode::~PaintNode() = default;

} // namespace VGG
