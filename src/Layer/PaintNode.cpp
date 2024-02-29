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
#include "Layer/Core/Transform.hpp"
#include "VSkia.hpp"
#include "Painter.hpp"
#include "PathGenerator.hpp"
#include "Renderer.hpp"
#include "PaintNodePrivate.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Shape.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/TreeNode.hpp"
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
#include <variant>

namespace VGG::layer
{

PaintNode::PaintNode(
  VRefCnt*           cnt,
  const std::string& name,
  EObjectType        type,
  const std::string& guid)
  : TreeNode(cnt, name)
  , d_ptr(new PaintNode__pImpl(this, type))
{
  d_ptr->guid = guid;
}

PaintNode::PaintNode(VRefCnt* cnt, const std::string& name, std::unique_ptr<PaintNode__pImpl> impl)
  : TreeNode(cnt, name)
  , d_ptr(std::move(impl))
{
}

void PaintNode::setContextSettings(const ContextSetting& settings)
{
  VGG_IMPL(PaintNode);
  _->contextSetting = settings;
}

Transform PaintNode::mapTransform(const PaintNode* node) const
{
  auto findPath = [](const TreeNode* node) -> std::vector<const TreeNode*>
  {
    std::vector<const TreeNode*> path = { node };
    while (node->parent())
    {
      node = node->parent().get();
      path.push_back(node);
    }
    return path;
  };
  auto            path1 = findPath(node);
  auto            path2 = findPath(this);
  const TreeNode* lca = nullptr;
  int             lcaIdx = -1;
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
    return Transform(mat);
  for (std::size_t i = 0; i < path1.size() && path1[i] != lca; i++)
  {
    auto skm = static_cast<const PaintNode*>(path1[i])->d_ptr->transform.matrix();
    auto inv = glm::inverse(skm);
    mat = mat * inv;
  }

  for (int i = lcaIdx - 1; i >= 0; i--)
  {
    const auto m = static_cast<const PaintNode*>(path2[i])->d_ptr->transform.matrix();
    mat = mat * m;
  }
  return Transform(mat);
}

ShapePath PaintNode::makeMaskBy(EBoolOp maskOp, Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  ShapePath result;
  if (_->maskedBy.empty())
    return result;

  // auto        op = toSkPathOp(maskOp);
  const auto& objects = renderer->maskObjects();
  for (const auto& id : _->maskedBy)
  {
    if (id != this->guid())
    {
      if (auto obj = objects.find(id); obj != objects.end())
      {
        const auto t = obj->second->mapTransform(this);
        auto       m = obj->second->asOutlineMask(&t);
        if (result.isEmpty())
        {
          result = m;
        }
        else
        {
          result.op(m, maskOp);
          // Op(result.outlineMask, m.outlineMask, op, &result.outlineMask);
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

void PaintNode::renderPass(Renderer* renderer)
{
  invokeRenderPass(renderer, 0);
}
void PaintNode::paintPass(Renderer* renderer, int zorder)
{
  VGG_IMPL(PaintNode);
  if (!_->path)
  {
    _->path = ShapePath(asOutlineMask(0));
  }
  if (_->path->isEmpty())
  {
    return;
  }
  if (!_->mask)
  {
    _->mask = makeMaskBy(BO_INTERSECTION, renderer);
  }

  _->ensureAlphaMask(renderer);

  // if (!_->alphaMask)
  // {
  //   MaskObject mo;
  //   auto       iter = _->alphaMaskBy.cbegin();
  //   auto       end = _->alphaMaskBy.cend();
  //   auto       comp = _->calcMaskObjects(renderer, iter, end, [](const auto& e) { return e.id;
  //   }); SkPath     path; for (const auto& e : comp)
  //   {
  //     if (path.isEmpty())
  //     {
  //       path = e.first->asOutlineMask(&e.second).outlineMask;
  //     }
  //     else
  //     {
  //       Op(
  //         path,
  //         e.first->asOutlineMask(&e.second).outlineMask,
  //         SkPathOp::kIntersect_SkPathOp,
  //         &path);
  //     }
  //   }
  //   if (!path.isEmpty())
  //   {
  //     mo.contour = std::move(path);
  //     mo.components = std::move(comp);
  //     _->alphaMask = std::move(mo);
  //   }
  // }

  this->paintEvent(renderer);
}

void PaintNode::paintEvent(Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  paintStyle(renderer, *_->path, *_->mask);
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

ShapePath PaintNode::makeBoundPath()
{
  const auto& skRect = toSkRect(frameBound());
  const auto& radius = style().frameRadius;
  bool        rounded = false;
  float       maxR = 0.0, minR = std::numeric_limits<float>::max();

  if (!radius.has_value())
  {
    return ShapePath(skRect);
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

  SkPath p;
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
  return ShapePath(p);
}

ShapePath PaintNode::childPolyOperation() const
{
  if (m_firstChild.empty())
  {
    WARN("no child in path %s", name().c_str());
    return ShapePath();
  }
  if (m_firstChild.size() == 1)
  {
    auto paintNode = static_cast<PaintNode*>(m_firstChild.front().get());
    return paintNode->asOutlineMask(&paintNode->transform());
  }

  std::vector<std::pair<ShapePath, EBoolOp>> ct;
  for (auto it = m_firstChild.begin(); it != m_firstChild.end(); ++it)
  {
    auto paintNode = static_cast<PaintNode*>(it->get());
    auto childMask = paintNode->asOutlineMask(&paintNode->transform());
    ct.emplace_back(childMask, paintNode->clipOperator());
  }

  if (ct.size() == 1)
  {
    return ct[0].first;
  }

  std::vector<ShapePath> res;

  // eval mask by operator
  ShapePath skPath = ct[0].first;
  for (std::size_t i = 1; i < ct.size(); i++)
  {
    ShapePath rhs;
    auto      op = ct[i].second;
    if (op != BO_NONE)
    {
      rhs = ct[i].first;
      skPath.op(rhs, op);
      // Op(skPath, rhs, skop, &skPath);
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
  for (const auto& s : res)
  {
    path.addPath(s.asPath());
  }
  return ShapePath(path);
}

ShapePath PaintNode::makeContourImpl(ContourOption option, const Transform* mat)
{
  VGG_IMPL(PaintNode);
  ShapePath path;
  if (_->contour)
  {
    // OPTIMIZE: cache the path to avoid recreate for everytime
    std::visit(
      Overloaded{
        [&](const Ellipse& c) { path.setOval(c); },
        [&](const ContourPtr& c) { path.setContour(c); },
        [&](const SkRect& c) { path.setRect(c); },
        [&](const SkRRect& c) { path.setRRect(c); },
      },
      *_->contour);
    // auto p = layer::makePath(*_->contour);
    if (mat)
    {
      ShapePath res;
      path.transform(res, toSkMatrix(mat->matrix()));
      return res;
    }
    return path;
  }

  auto appendPath = [this](ShapePath& path, const ContourOption& option, EBoolOp op)
  {
    for (auto it = begin(); it != end(); ++it)
    {
      auto paintNode = static_cast<PaintNode*>(it->get());
      auto childMask = paintNode->makeContourImpl(option, &paintNode->transform());
      path.op(childMask, op);
    }
  };

  switch (option.contourType)
  {
    case MCT_FRAMEONLY:
      path = this->makeBoundPath();
      break;
    case MCT_UNION_WITH_FRAME:
      path = this->makeBoundPath();
    case MCT_UNION:
      appendPath(path, option, BO_UNION);
      break;
    case MCT_INTERSECT_WITH_FRAME:
      path = this->makeBoundPath();
    case MCT_INTERSECT:
      appendPath(path, option, BO_INTERSECTION);
      break;
    case MCT_OBJECT_OPS:
      path = childPolyOperation();
      break;
  }
  if (mat)
  {
    ShapePath res;
    path.transform(res, toSkMatrix(mat->matrix()));
    return res;
  }
  return path;
}

void PaintNode::drawAsAlphaMask(Renderer* renderer, sk_sp<SkBlender> blender)
{
  VGG_IMPL(PaintNode);
  if (_->contextSetting.opacity < 1.0)
  {
    renderer->canvas()->saveLayerAlpha(0, _->contextSetting.opacity * 255);
  }
  d_ptr->drawAsAlphaMaskImpl(renderer, std::move(blender));
  if (_->contextSetting.opacity < 1.0)
  {
    renderer->canvas()->restore();
  }
}

void PaintNode::drawRawStyle(Painter& painter, const ShapePath& path, sk_sp<SkBlender> blender)
{
  d_ptr->drawRawStyleImpl(painter, path, std::move(blender));
}

ShapePath PaintNode::asOutlineMask(const Transform* mat)
{
  ShapePath mask;
  mask = makeContourImpl(maskOption(), mat);
  mask.setFillType(childWindingType());
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
void PaintNode::setTransform(const Transform& transform)
{
  VGG_IMPL(PaintNode);
  _->transform = transform;
}

Transform& PaintNode::transform()
{
  return d_ptr->transform;
}

const Transform& PaintNode::transform() const
{
  return d_ptr->transform;
}

Transform PaintNode::globalTransform() const
{
  auto mat = glm::identity<glm::mat3>();
  d_ptr->worldTransform(mat);
  return Transform(mat);
}

const Bound& PaintNode::frameBound() const
{
  return d_ptr->bound;
}

void PaintNode::setFrameBound(const Bound& bound)
{
  VGG_IMPL(PaintNode);
  _->bound = bound;
  invalidate();
}

Bound PaintNode::onRevalidate()
{
  for (const auto& e : m_firstChild)
  {
    e->revalidate();
  }
  return d_ptr->bound;
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

void PaintNode::setContourData(ContourData contour)
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

void PaintNode::invokeRenderPass(Renderer* renderer, int zorder)
{
  VGG_IMPL(PaintNode);
  if (!_->visible)
    return;
  if (_->paintOption.paintStrategy == EPaintStrategy::PS_SELFONLY)
  {
    prePaintPass(renderer);
    paintPass(renderer, zorder);
    postPaintPass(renderer);
  }
  else if (_->paintOption.paintStrategy == EPaintStrategy::PS_RECURSIVELY)
  {
    prePaintPass(renderer);
    paintPass(renderer, zorder);
    paintChildrenPass(renderer);
    postPaintPass(renderer);
  }
  else if (_->paintOption.paintStrategy == EPaintStrategy::PS_CHILDONLY)
  {
    prePaintPass(renderer);
    paintChildrenPass(renderer);
    postPaintPass(renderer);
  }
}

void PaintNode::paintChildrenRecursively(Renderer* renderer)
{

  VGG_IMPL(PaintNode);
  std::vector<PaintNode*> masked;
  std::vector<PaintNode*> noneMasked;
  auto                    canvas = renderer->canvas();
  for (const auto& p : this->m_firstChild)
  {
    auto c = static_cast<PaintNode*>(p.get());
    if (c->maskType() == MT_OUTLINE)
    {
      if (c->d_ptr->maskShowType == MST_CONTENT)
      {
        masked.push_back(c);
      }
    }
    if (c->maskType() == MT_NONE)
    {
      noneMasked.push_back(c);
    }
  }

  int  zorder = 0;
  auto paintCall = [&](std::vector<PaintNode*>& nodes)
  {
    if (_->contextSetting.transparencyKnockoutGroup)
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

  if (overflow() == OF_HIDDEN)
  {
    canvas->save();
    auto boundPath = makeBoundPath();
    boundPath.clip(canvas, SkClipOp::kIntersect);
    // canvas->clipPath(makeBoundPath());
  }
  paintCall(masked);
  paintCall(noneMasked);
  if (overflow() == OF_HIDDEN)
  {
    canvas->restore();
  }
}

void PaintNode::paintChildrenPass(Renderer* renderer)
{
  paintChildrenRecursively(renderer);
}
void PaintNode::prePaintPass(Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  auto canvas = renderer->canvas();
  _->layerContextGuard.saveLayer(
    _->contextSetting,
    [&](const SkPaint& p)
    {
      // const auto b = toSkRect(bound());
      canvas->saveLayer(0, &p);
    });

  canvas->save();
  canvas->concat(toSkMatrix(_->transform.matrix()));
  if (renderer->isEnableDrawDebugBound())
  {
    renderer->drawDebugBound(this, 0);
  }
}

void PaintNode::postPaintPass(Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  auto canvas = renderer->canvas();
  canvas->restore();
  _->layerContextGuard.restore([&]() { canvas->restore(); });
}

void PaintNode::paintStyle(Renderer* renderer, const ShapePath& path, const ShapePath& outlineMask)
{
  VGG_IMPL(PaintNode);
  Painter painter(renderer);
  // draw blur, we assume that there is only one blur style
  Blur    blur;
  auto    blurType = _->blurType(blur);
  if (!_->alphaMask)
  {
    // 1. normal drawing
    if (blurType)
    {
      ShapePath res = path;
      if (!outlineMask.isEmpty())
        res.op(outlineMask, EBoolOp::BO_INTERSECTION);
      // Op(res, outlineMask, SkPathOp::kIntersect_SkPathOp, &res);
      if (*blurType == BT_GAUSSIAN)
        painter.blurContentBegin(blur.radius, blur.radius, frameBound(), nullptr, 0);
      else if (*blurType == BT_BACKGROUND)
        painter.blurBackgroundBegin(blur.radius, blur.radius, frameBound(), &res);
      else if (*blurType == BT_MOTION)
        DEBUG("Motion blur has not been implemented");
      else if (*blurType == BT_ZOOM)
        DEBUG("Zoom blur has not been implemented");
    }
    if (!outlineMask.isEmpty())
    {
      // painter.beginClip(outlineMask); // TODO::
      painter.canvas()->save();
      outlineMask.clip(painter.renderer()->canvas(), SkClipOp::kIntersect);
    }
    auto blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    drawRawStyle(painter, path, blender);
    if (!outlineMask.isEmpty())
    {
      painter.canvas()->restore();
    }
    if (blurType)
    {
      // const auto bt = blurType.value();
      if (*blurType == BT_GAUSSIAN)
        painter.blurContentEnd();
      else if (*blurType == BT_BACKGROUND)
        painter.blurBackgroundEnd();
      else if (*blurType == BT_ZOOM)
        DEBUG("Zoom blur has not been implemented");
      else if (*blurType == BT_MOTION)
        DEBUG("Motion blur has not been implemented");
    }
  }
  else
  {
    // 1. only alphamask: alphamask layer
    // 2. alphamask + background blur: blured content layer
    // 3. alphamask + content blur: alphamask layer + content blur layer
    if (!blurType)
      return _->drawWithAlphaMask(renderer, path, outlineMask);
    else if (*blurType == BT_BACKGROUND)
      return _->drawBlurBgWithAlphaMask(renderer, path, outlineMask);
    else if (*blurType == BT_GAUSSIAN)
      return _->drawBlurContentWithAlphaMask(renderer, path, outlineMask);
  }
}

void PaintNode::paintFill(
  Renderer*            renderer,
  sk_sp<SkBlender>     blender,
  sk_sp<SkImageFilter> imageFilter,
  const ShapePath&     path)
{
  Painter             painter(renderer);
  sk_sp<SkMaskFilter> blur;
  for (size_t i = 0; i < style().fills.size(); ++i)
  {
    auto& f = style().fills[i];
    if (!f.isEnabled)
      continue;
    painter.drawFill(path, frameBound(), f, imageFilter, blender, blur);
  }
}

PaintNode::~PaintNode() = default;

} // namespace VGG::layer
