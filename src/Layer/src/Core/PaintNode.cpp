#include "Core/PaintNode.h"
#include "Core/Node.hpp"
#include "Core/VType.h"
#include "SkiaImpl/VSkia.h"
#include "core/SkCanvas.h"
#include <core/SkPath.h>
#include <core/SkRRect.h>

namespace VGG
{

SkCanvas* PaintNode::s_defaultCanvas = nullptr;
RenderState* PaintNode::s_renderState = nullptr;

class PaintNode__pImpl
{
  VGG_DECL_API(PaintNode);

public:
  PaintNode__pImpl(PaintNode* api)
    : q_ptr(api)
  {
  }
};

PaintNode::PaintNode(const std::string& name, ObjectType type)
  : Node(name)
  , m_type(type)
  , d_ptr(new PaintNode__pImpl(this))
{
}

glm::mat3 PaintNode::mapTransform(const PaintNode* node) const
{
  auto find_path = [](const Node* node) -> std::vector<const Node*>
  {
    std::vector<const Node*> path = { node };
    while (node->parent())
    {
      node = node->parent().get();
      path.push_back(node);
    }
    return path;
  };
  auto path1 = find_path(node);
  auto path2 = find_path(this);
  const Node* lca = nullptr;
  int lca_idx = -1;
  for (int i = path1.size() - 1, j = path2.size() - 1; i >= 0 && j >= 0; i--, j--)
  {
    auto n1 = path1[i];
    auto n2 = path2[j];
    if (n1 == n2)
    {
      lca = n1;
      lca_idx = j;
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
    auto skm = static_cast<const PaintNode*>(path1[i])->m_transform;
    auto inv = glm::inverse(skm);
    mat = mat * inv;
  }

  for (int i = lca_idx - 1; i >= 0; i--)
  {
    const auto m = static_cast<const PaintNode*>(path2[i])->m_transform;
    mat = mat * m;
  }
  return mat;
}

Mask PaintNode::makeMaskBy(EBoolOp maskOp)
{
  Mask result;
  if (m_maskedBy.empty())
    return result;

  auto op = toSkPathOp(maskOp);
  auto objects = Scene::getObjectTable();
  for (const auto id : m_maskedBy)
  {
    if (id != this->GUID())
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
  const auto& b = getBound();
  SkPaint strokePen;
  strokePen.setStyle(SkPaint::kStroke_Style);
  SkColor color = nodeType2Color(this->m_type);
  strokePen.setColor(color);
  strokePen.setStrokeWidth(2);
  canvas->drawRect(toSkRect(getBound()), strokePen);
}
void PaintNode::visitNode(VGG::Node* p, ObjectTableType& table)
{
  if (!p)
    return;
  auto sptr = std::static_pointer_cast<PaintNode>(p->shared_from_this());
  if (sptr->m_maskType != MT_None)
  {
    if (auto it = table.find(sptr->GUID()); it == table.end())
    {
      table[sptr->GUID()] = sptr; // type of all children of paintnode must be paintnode
    }
  }
  for (auto it = p->begin(); it != p->end(); ++it)
  {
    visitNode(it->get(), table);
  }
}

void PaintNode::paintPass()
{
  SkCanvas* canvas = getSkCanvas();
  canvas->save();
  canvas->concat(toSkMatrix(this->m_transform));
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
  const auto& bound = toSkRect(getBound());
  const auto radius = style().frameRadius;
  SkRRect rrect;
  if (radius > 0.0)
  {
    rrect = SkRRect::MakeRectXY(bound, radius, radius);
  }
  if (overflow() == EOverflow::OF_Hidden)
  {
    if (radius > 0.0)
    {
      canvas->clipRRect(rrect, true);
    }
    else
    {
      canvas->clipRect(bound);
    }
  }
  if (this->m_bgColor.has_value())
  {
    SkPaint bgPaint;
    bgPaint.setColor(this->m_bgColor.value());
    bgPaint.setStyle(SkPaint::kFill_Style);
    if (radius > 0.0)
    {
      canvas->drawRRect(rrect, bgPaint);
    }
    else
    {
      canvas->drawRect(bound, bgPaint);
    }
  }
}

SkPath PaintNode::makeBoundMask()
{
  SkPath p;
  const auto& skRect = toSkRect(getBound());
  const auto r = style().frameRadius;
  if (style().frameRadius > 0)
  {
    p.addRoundRect(skRect, r, r);
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
    path = makeBoundMask();
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
        auto childMask =
          paintNode->makeOutlineMask(EMaskCoutourType::MCT_Union, &paintNode->localTransform());
        Op(path, childMask, SkPathOp::kUnion_SkPathOp, &path);
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
        auto childMask =
          paintNode->makeOutlineMask(EMaskCoutourType::MCT_Union, &paintNode->localTransform());
        Op(path, childMask, SkPathOp::kIntersect_SkPathOp, &path);
      }
    }
    break;
    case MCT_UnionDependsOn:
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
  m_outlineMask = mask;
}

void PaintNode::asAlphaMask()
{
}

PaintNode::~PaintNode() = default;

} // namespace VGG
