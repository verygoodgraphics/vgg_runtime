#include "Core/PaintNode.h"
#include "Core/Node.hpp"
#include "Core/VGGType.h"
#include "Core/VGGUtils.h"
#include "SkiaBackend/SkiaImpl.h"
#include "core/SkCanvas.h"
#include <core/SkPath.h>

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
  , type(type)
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
  if (maskedBy.empty())
    return result;

  auto op = toSkPathOp(maskOp);
  auto objects = Scene::getObjectTable();
  for (const auto id : maskedBy)
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
  SkColor color = nodeType2Color(this->type);
  strokePen.setColor(color);
  strokePen.setStrokeWidth(2);
  canvas->drawRect(toSkRect(getBound()), strokePen);
}
void PaintNode::visitNode(VGG::Node* p, ObjectTableType& table)
{
  if (!p)
    return;
  auto sptr = std::static_pointer_cast<PaintNode>(p->shared_from_this());
  if (sptr->maskType != MT_None)
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
  if (overflow() == EOverflow::OF_Hidden)
  {
    canvas->clipRect(toSkRect(getBound()));
  }
  if (this->bgColor.has_value())
  {
    SkPaint bgPaint;
    bgPaint.setColor(this->bgColor.value());
    bgPaint.setStyle(SkPaint::kFill_Style);
    canvas->drawRect(toSkRect(getBound()), bgPaint);
  }
}

Mask PaintNode::asOutlineMask(const glm::mat3* mat)
{
  SkPath p;
  Mask mask;
  p.addRect(toSkRect(getBound()));
  if (mat)
  {
    p.transform(toSkMatrix(*mat));
  }
  mask.outlineMask = p;
  return mask;
}

void PaintNode::setOutlineMask(const Mask& mask)
{
  outlineMask = mask;
}

void PaintNode::asAlphaMask()
{
}

PaintNode::~PaintNode() = default;

} // namespace VGG
