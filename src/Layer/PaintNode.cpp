/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "VSkia.hpp"
#include "Painter.hpp"
#include "Renderer.hpp"
#include "RenderState.hpp"
#include "PaintNodePrivate.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Node.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <core/SkBlendMode.h>
#include <core/SkBlurTypes.h>
#include <core/SkColor.h>
#include <core/SkColorFilter.h>
#include <core/SkImageFilter.h>
#include <core/SkImageInfo.h>
#include <core/SkMatrix.h>
#include <core/SkPaint.h>
#include <core/SkRect.h>
#include <core/SkSamplingOptions.h>
#include <core/SkSurfaceProps.h>
#include <core/SkTileMode.h>
#include <core/SkMaskFilter.h>
#include <src/core/SkBlurMask.h>
#include <src/core/SkMask.h>
#include <effects/SkImageFilters.h>
#include <effects/SkRuntimeEffect.h>
#include <encode/SkPngEncoder.h>
#include <gpu/GrDirectContext.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <core/SkPath.h>
#include <core/SkRRect.h>
#include <optional>
#include <pathops/SkPathOps.h>
#include <src/core/SkRuntimeEffectPriv.h>
#include <limits>
#include <fstream>
#include <string_view>

namespace VGG
{

PaintNode::PaintNode(const std::string& name, ObjectType type, const std::string& guid)
  : Node(name)
  , d_ptr(new PaintNode__pImpl(this, type))
{
  d_ptr->guid = guid;
}

PaintNode::PaintNode(const std::string& name, std::unique_ptr<PaintNode__pImpl> impl)
  : Node(name)
  , d_ptr(std::move(impl))
{
}

PaintNode::PaintNode(const PaintNode& other)
  : Node(other.name())
  , d_ptr(new PaintNode__pImpl(*other.d_ptr))
{
}

NodePtr PaintNode::clone() const
{
  auto newNode = std::make_shared<PaintNode>(*this);
  return newNode;
}

void PaintNode::setContextSettings(const ContextSetting& settings)
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

Mask PaintNode::makeMaskBy(EBoolOp maskOp, SkiaRenderer* renderer)
{
  VGG_IMPL(PaintNode);
  Mask result;
  if (_->maskedBy.empty())
    return result;

  auto op = toSkPathOp(maskOp);
  const auto& objects = renderer->maskObjects();
  for (const auto id : _->maskedBy)
  {
    if (id != this->guid())
    {
      if (auto obj = objects.find(id); obj != objects.end())
      {
        const auto t = obj->second->mapTransform(this);
        auto m = obj->second->asOutlineMask(&t);
        if (result.outlineMask.isEmpty())
        {
          result = m;
        }
        else
        {
          Op(result.outlineMask, m.outlineMask, op, &result.outlineMask);
        }
      }
      else
      {
        DEBUG("No such mask: %s", id.c_str());
      }
    }
  }
  return result;
}

void PaintNode::renderPass(SkiaRenderer* renderer)
{
  invokeRenderPass(renderer, 0);
}
void PaintNode::paintPass(SkiaRenderer* renderer, int zorder)
{
  VGG_IMPL(PaintNode);
  if (!_->path)
  {
    _->path = asOutlineMask(0).outlineMask;
  }
  if (_->path->isEmpty())
  {
    return;
  }
  if (!_->mask)
  {
    _->mask = makeMaskBy(BO_Intersection, renderer).outlineMask;
  }

  if (!_->alphaMask)
  {
    MaskObject mo;
    auto iter = _->alphaMaskBy.cbegin();
    auto end = _->alphaMaskBy.cend();
    auto comp = _->calcMaskObjects(renderer, iter, end, [](const auto& e) { return e.id; });
    SkPath path;
    for (const auto& e : comp)
    {
      if (path.isEmpty())
      {
        path = e.first->asOutlineMask(&e.second).outlineMask;
      }
      else
      {
        Op(path,
           e.first->asOutlineMask(&e.second).outlineMask,
           SkPathOp::kIntersect_SkPathOp,
           &path);
      }
    }
    if (!path.isEmpty())
    {
      mo.contour = std::move(path);
      mo.components = std::move(comp);
      _->alphaMask = std::move(mo);
    }
  }

  // renderer->displayList.emplace_back(renderer->canvas()->getTotalMatrix(), this);
  // renderer->pushItem(this, zorder);
  this->paintEvent(renderer);
}

void PaintNode::paintEvent(SkiaRenderer* renderer)
{
  VGG_IMPL(PaintNode);
  // const auto path = stylePath();
  // if (!_->maskObjects)
  // {
  //   _->maskObjects = _->calcMaskObjects(renderer);
  // }
  paintStyle(renderer->canvas(), *_->path, *_->mask);
}

void PaintNode::setMaskBy(std::vector<std::string> masks)
{
  VGG_IMPL(PaintNode);
  _->maskedBy = std::move(masks);
}

void PaintNode::setAlphaMaskBy(std::vector<AlphaMask> masks)
{
  VGG_IMPL(PaintNode);
  _->alphaMaskBy = std::move(masks);
}

SkPath PaintNode::makeBoundPath()
{
  SkPath p;
  const auto& skRect = toSkRect(getBound());
  const auto& radius = style().frameRadius;

  bool rounded = false;
  float maxR = 0.0, minR = std::numeric_limits<float>::max();

  if (!radius.has_value())
  {
    p.addRect(skRect);
    return p;
  }

  for (const auto r : *radius)
  {
    if (r > 0.f)
    {
      rounded = true;
    }
    maxR = std::max(maxR, r);
    minR = std::min(minR, r);
  }
  if (rounded)
  {
    if ((maxR - minR) < std::numeric_limits<float>::epsilon())
    {
      p.addRoundRect(skRect, minR, minR);
    }
    else
    {
      // TODO:: create general path
      p.addRoundRect(skRect, minR, minR);
    }
  }
  else
  {
    p.addRect(skRect);
  }
  return p;
}

SkPath PaintNode::childPolyOperation() const
{
  if (m_firstChild.empty())
  {
    WARN("no child in path %s", name().c_str());
    return SkPath();
  }
  std::vector<std::pair<SkPath, EBoolOp>> ct;
  for (auto it = m_firstChild.cbegin(); it != m_firstChild.cend(); ++it)
  {
    auto paintNode = static_cast<PaintNode*>(it->get());
    auto childMask = paintNode->asOutlineMask(&paintNode->localTransform());
    ct.emplace_back(childMask.outlineMask, paintNode->clipOperator());
  }

  if (ct.size() == 1)
  {
    return ct[0].first;
  }

  std::vector<SkPath> res;

  // eval mask by operator
  SkPath skPath = ct[0].first;
  for (int i = 1; i < ct.size(); i++)
  {
    SkPath rhs;
    auto op = ct[i].second;
    if (op != BO_None)
    {
      auto skop = toSkPathOp(op);
      rhs = ct[i].first;
      Op(skPath, rhs, skop, &skPath);
    }
    else
    {
      res.push_back(skPath);
      skPath = ct[i].first;
    }
    op = ct[i].second;
  }
  res.push_back(skPath);

  SkPath path;
  for (const auto s : res)
  {
    path.addPath(s);
  }
  return path;
}

SkPath PaintNode::makeContourImpl(ContourOption option, const glm::mat3* mat)
{
  VGG_IMPL(PaintNode);
  SkPath path;
  if (_->contour)
  {
    path = getSkiaPath(*_->contour, _->contour->closed);
    if (mat)
    {
      path.transform(toSkMatrix(*mat));
    }
    return path;
  }

  auto appendPath = [this](SkPath& path, const ContourOption& option, SkPathOp op)
  {
    for (auto it = begin(); it != end(); ++it)
    {
      auto paintNode = static_cast<PaintNode*>(it->get());
      auto childMask = paintNode->makeContourImpl(option, &paintNode->localTransform());
      Op(path, childMask, op, &path);
    }
  };

  switch (option.contourType)
  {
    case MCT_FrameOnly:
      path = this->makeBoundPath();
      break;
    case MCT_UnionWithFrame:
      path = this->makeBoundPath();
    case MCT_Union:
      appendPath(path, option, SkPathOp::kUnion_SkPathOp);
      break;
    case MCT_IntersectWithFrame:
      path = this->makeBoundPath();
    case MCT_Intersect:
      appendPath(path, option, SkPathOp::kIntersect_SkPathOp);
      break;
    case MCT_ByObjectOps:
      path = childPolyOperation();
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
  VGG_IMPL(PaintNode);
  Mask mask;
  mask.outlineMask = makeContourImpl(maskOption(), mat);
  mask.outlineMask.setFillType(childWindingType() == EWindingType::WR_EvenOdd
                                 ? SkPathFillType::kEvenOdd
                                 : SkPathFillType::kWinding);
  return mask;
}

void PaintNode::setOutlineMask(const Mask& mask)
{
  VGG_IMPL(PaintNode);
  _->outlineMask = mask;
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

void PaintNode::setChildWindingType(EWindingType rule)
{
  VGG_IMPL(PaintNode);
  _->windingRule = rule;
}

EWindingType PaintNode::childWindingType() const
{
  return d_ptr->windingRule;
}

void PaintNode::setVisible(bool visible)
{
  VGG_IMPL(PaintNode);
  _->visible = visible;
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

glm::mat3 PaintNode::worldTransform() const
{
  auto mat = glm::identity<glm::mat3>();
  d_ptr->worldTransform(mat);
  return mat;
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

void PaintNode::setMaskShowType(EMaskShowType type)
{
  VGG_IMPL(PaintNode);
  _->maskShowType = type;
}

void PaintNode::setContourOption(ContourOption option)
{
  VGG_IMPL(PaintNode);
  _->maskOption = option;
}

void PaintNode::setContourData(ContourPtr contour)
{
  VGG_IMPL(PaintNode);
  _->contour = std::move(contour);
}

const ContourOption& PaintNode::maskOption() const
{
  return d_ptr->maskOption;
}

void PaintNode::setPaintOption(PaintOption option)
{
  VGG_IMPL(PaintNode);
  _->paintOption = option;
}

const PaintOption& PaintNode::paintOption() const
{
  return d_ptr->paintOption;
}

void PaintNode::invokeRenderPass(SkiaRenderer* renderer, int zorder)
{
  VGG_IMPL(PaintNode);
  if (!_->visible)
    return;
  if (_->paintOption.paintStrategy == EPaintStrategy::PS_SelfOnly)
  {
    prePaintPass(renderer);
    paintPass(renderer, zorder);
    postPaintPass(renderer);
  }
  else if (_->paintOption.paintStrategy == EPaintStrategy::PS_Recursively)
  {
    prePaintPass(renderer);
    paintPass(renderer, zorder);
    paintChildrenPass(renderer);
    postPaintPass(renderer);
  }
  else if (_->paintOption.paintStrategy == EPaintStrategy::PS_ChildOnly)
  {
    prePaintPass(renderer);
    paintChildrenPass(renderer);
    postPaintPass(renderer);
  }
}

void PaintNode::paintChildrenRecursively(SkiaRenderer* renderer)
{

  VGG_IMPL(PaintNode);
  std::vector<PaintNode*> masked;
  std::vector<PaintNode*> noneMasked;
  auto canvas = renderer->canvas();
  for (const auto& p : this->m_firstChild)
  {
    auto c = static_cast<PaintNode*>(p.get());
    if (c->maskType() == MT_Outline)
    {
      if (c->d_ptr->maskShowType == MST_Content)
      {
        masked.push_back(c);
      }
    }
    if (c->maskType() == MT_None)
    {
      noneMasked.push_back(c);
    }
  }

  int zorder = 0;
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
        //
        p->invokeRenderPass(renderer, zorder);
        // canvas->restore();
        // canvas->restore();
        zorder++;
      }
    }
    else
    {
      for (const auto& p : nodes)
      {
        p->invokeRenderPass(renderer, zorder);
        zorder++;
      }
    }
  };

  if (overflow() == OF_Hidden)
  {
    canvas->save();
    canvas->clipPath(makeBoundPath());
  }
  paintCall(masked);
  paintCall(noneMasked);
  if (overflow() == OF_Hidden)
  {
    canvas->restore();
  }
}

void PaintNode::paintChildrenPass(SkiaRenderer* renderer)
{
  VGG_IMPL(PaintNode);
  paintChildrenRecursively(renderer);
}
void PaintNode::prePaintPass(SkiaRenderer* renderer)
{
  VGG_IMPL(PaintNode);
  auto canvas = renderer->canvas();
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

  canvas->save();
  canvas->concat(toSkMatrix(_->transform));
  if (renderer->isEnableDrawDebugBound())
  {
    renderer->drawDebugBound(this, 0);
  }
  // renderer->pushMatrix(this->localTransform());
}

void PaintNode::postPaintPass(SkiaRenderer* renderer)
{
  VGG_IMPL(PaintNode);
  auto canvas = renderer->canvas();
  // renderer->popMatrix();
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
  return asOutlineMask(0).outlineMask;

  std::vector<std::pair<SkPath, EBoolOp>> ct;
  for (const auto& c : m_firstChild)
  {
    auto p = static_cast<PaintNode*>(c.get());
    auto outline = p->asOutlineMask(&p->localTransform());
    ct.emplace_back(outline.outlineMask, p->clipOperator());
  }
  if (m_firstChild.empty())
  {
    ct.emplace_back(asOutlineMask(0).outlineMask, EBoolOp::BO_None);
  }
  assert(ct.size() >= 1);

  if (ct.size() == 1)
  {
    return ct[0].first;
  }

  std::vector<SkPath> res;
  SkPath skPath = ct[0].first;
  for (int i = 1; i < ct.size(); i++)
  {
    SkPath rhs;
    auto op = ct[i].second;
    if (op != BO_None)
    {
      auto skop = toSkPathOp(op);
      rhs = ct[i].first;
      Op(skPath, rhs, skop, &skPath);
    }
    else
    {
      res.push_back(skPath);
      skPath = ct[i].first;
    }
    op = ct[i].second;
  }
  res.push_back(skPath);

  for (const auto s : res)
  {
    skPath.addPath(s);
  }
  skPath.setFillType(childWindingType() == EWindingType::WR_EvenOdd ? SkPathFillType::kEvenOdd
                                                                    : SkPathFillType::kWinding);

  return skPath;
}

void PaintNode::paintStyle(SkCanvas* canvas, const SkPath& path, const SkPath& outlineMask)
{
  VGG_IMPL(PaintNode);
  Painter painter(canvas);
  // draw blur, we assume that there is only one blur style
  Blur blur;
  auto blurType = _->blurType(blur);
  if (!_->alphaMask)
  {
    // 1. normal drawing
    if (blurType)
    {
      SkPath res = path;
      if (!outlineMask.isEmpty())
        Op(res, outlineMask, SkPathOp::kIntersect_SkPathOp, &res);
      if (*blurType == BT_Gaussian)
        painter.blurContentBegin(blur.radius, blur.radius, getBound(), nullptr, 0);
      else if (*blurType == BT_Background)
        painter.blurBackgroundBegin(blur.radius, blur.radius, getBound(), &res);
      else if (*blurType == BT_Motion)
        DEBUG("Motion blur has not been implemeted");
      else if (*blurType == BT_Zoom)
        DEBUG("Zoom blur has not been implemeted");
    }
    if (!outlineMask.isEmpty())
    {
      painter.beginClip(outlineMask);
    }
    auto blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    _->drawRawStyle(painter, path, blender);
    if (!outlineMask.isEmpty())
    {
      painter.endClip();
    }
    if (blurType)
    {
      const auto bt = blurType.value();
      if (*blurType == BT_Gaussian)
        painter.blurContentEnd();
      else if (*blurType == BT_Background)
        painter.blurBackgroundEnd();
      else if (*blurType == BT_Zoom)
        DEBUG("Zoom blur has not been implemeted");
      else if (*blurType == BT_Motion)
        DEBUG("Zoom blur has not been implemeted");
    }
  }
  else
  {
    // 1. only alphamask: alphamask layer
    // 2. alphamask + background blur: blured content layer
    // 3. alphamask + content blur: alphamask layer + content blur layer
    if (!blurType)
      return _->drawWithAlphaMask(canvas, path, outlineMask);
    else if (*blurType == BT_Background)
      return _->drawBlurBgWithAlphaMask(canvas, path, outlineMask);
    else if (*blurType == BT_Gaussian)
      return _->drawBlurContentWithAlphaMask(canvas, path, outlineMask);
  }
}

void PaintNode::paintFill(SkCanvas* canvas, sk_sp<SkBlender> blender, const SkPath& path)
{
  VGG_IMPL(PaintNode);
  Painter render(canvas);
  sk_sp<SkMaskFilter> blur;
  for (const auto& f : style().fills)
  {
    if (!f.isEnabled)
      continue;
    render.drawFill(path, getBound(), f, contextSetting().Opacity, 0, blender, blur);
  }
}

PaintNode::~PaintNode() = default;

} // namespace VGG
