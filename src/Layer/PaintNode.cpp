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
#include "Layer/Guard.hpp"
#include "Layer/SkSL.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "PaintNodePrivate.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Effects.hpp"
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

void PaintNode::render(Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  if (!_->visible)
    return;
  auto canvas = renderer->canvas();
  {
    SaveLayerContextGuard lcg(
      canvas,
      _->contextSetting,
      [&](SkCanvas* canvas, const SkPaint& p) { canvas->saveLayer(0, &p); });
    {
      SkAutoCanvasRestore acr(canvas, true);
      canvas->concat(toSkMatrix(_->transform.matrix()));
      if (renderer->isEnableDrawDebugBound())
      {
        renderer->drawDebugBound(this, 0);
      }

      if (_->paintOption.paintStrategy == EPaintStrategy::PS_SELFONLY)
      {
        paintSelf(renderer);
      }
      else if (_->paintOption.paintStrategy == EPaintStrategy::PS_RECURSIVELY)
      {
        paintSelf(renderer);
        paintChildren(renderer);
      }
      else if (_->paintOption.paintStrategy == EPaintStrategy::PS_CHILDONLY)
      {
        paintChildren(renderer);
      }
    }
  }
}
void PaintNode::paintSelf(Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  if (!_->path)
  {
    _->path = VShape(asVisualShape(0));
  }
  if (_->path->isEmpty())
  {
    return;
  }
  this->paintEvent(renderer);
}

void PaintNode::paintEvent(Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  auto dropbackFilter = _->backgroundBlurImageFilter();
  auto layerFilter = _->blurImageFilter();

  const auto newLayer = dropbackFilter || layerFilter || !_->alphaMaskBy.empty();
  ASSERT(_->path);
  auto&  path = *_->path;
  VShape shapeMask;
  if (!_->maskedBy.empty())
  {
    auto iter = ShapeMaskIterator(_->maskedBy);
    shapeMask =
      MaskBuilder::makeShapeMask(this, renderer->maskObjects(), iter, toSkRect(frameBound()), 0);
  }

  /// ====
  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->save();
    shapeMask.clip(renderer->canvas(), SkClipOp::kIntersect);
  }

  SkPaint lp;
  lp.setStyle(SkPaint::kStroke_Style);
  if (newLayer)
  {
    SkRect objectBound;
    _->ensureStyleObjectRecorder(path, shapeMask, 0, _->style.fills, _->style.borders);
    objectBound.join(_->styleDisplayList->bounds());
    _->ensureDropShadowEffects(_->style.dropShadow, path);
    objectBound.join(_->dropShadowEffects->bounds());
    SkRect layerBound =
      layerFilter ? layerFilter.get()->computeFastBounds(objectBound) : objectBound;

    if (!_->alphaMaskBy.empty())
    {
      auto alphaMaskIter = AlphaMaskIterator(_->alphaMaskBy);
      layerFilter = MaskBuilder::makeAlphaMaskWith(
        layerFilter,
        this,
        renderer->maskObjects(),
        alphaMaskIter,
        layerBound,
        0);
    }
    if (dropbackFilter)
    {
      if (auto df = _->styleDisplayList->asImageFilter(); df)
      {
        auto blender = getOrCreateBlender("maskOut", g_maskOutBlender);
        dropbackFilter =
          SkImageFilters::Blend(blender, df, dropbackFilter, _->styleDisplayList->bounds());
      }
    }
    SkPaint layerPaint;
    layerPaint.setAntiAlias(true);
    layerPaint.setImageFilter(layerFilter);
    VShape clipShape(layerBound);
    _->beginLayer(renderer, &layerPaint, &clipShape, dropbackFilter);
    if (renderer->isEnableDrawDebugBound())
    {
      lp.setColor(SK_ColorBLUE);
      lp.setStrokeWidth(2);
      renderer->canvas()->drawRect(layerBound, lp);
      lp.setColor(SK_ColorCYAN);
      lp.setStrokeWidth(4);
      renderer->canvas()->drawRect(_->dropShadowEffects->bounds(), lp);
      lp.setColor(SK_ColorGREEN);
      lp.setStrokeWidth(3);
      renderer->canvas()->drawRect(_->styleDisplayList->bounds(), lp);
    }
  }

  onDrawStyle(renderer, path, shapeMask, 0);
  if (newLayer)
  {
    _->endLayer(renderer);
  }

  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->restore();
  }
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

VShape PaintNode::makeBoundPath()
{
  const auto& skRect = toSkRect(frameBound());
  return std::visit(
    Overloaded{
      [&](const ContourPtr& c) { return VShape(c); },
      [&](const SkRect& r) { return VShape(r); },
      [&](const SkRRect& r) { return VShape(r); },
    },
    makeShape(style().frameRadius, skRect, style().cornerSmooth));
}

VShape PaintNode::childPolyOperation() const
{
  if (m_firstChild.empty())
  {
    WARN("no child in path %s", name().c_str());
    return VShape();
  }
  if (m_firstChild.size() == 1)
  {
    auto paintNode = static_cast<PaintNode*>(m_firstChild.front().get());
    return paintNode->asVisualShape(&paintNode->transform());
  }

  std::vector<std::pair<VShape, EBoolOp>> ct;
  for (auto it = m_firstChild.begin(); it != m_firstChild.end(); ++it)
  {
    auto paintNode = static_cast<PaintNode*>(it->get());
    auto childMask = paintNode->asVisualShape(&paintNode->transform());
    ct.emplace_back(childMask, paintNode->clipOperator());
  }

  std::vector<VShape> res;

  // eval mask by operator
  VShape skPath = ct[0].first;
  for (std::size_t i = 1; i < ct.size(); i++)
  {
    VShape rhs;
    auto   op = ct[i].second;
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
  return VShape(path);
}

VShape PaintNode::makeContourImpl(ContourOption option, const Transform* mat)
{
  VGG_IMPL(PaintNode);
  VShape path;
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
      VShape res;
      path.transform(res, toSkMatrix(mat->matrix()));
      return res;
    }
    return path;
  }

  auto appendPath = [this](VShape& path, const ContourOption& option, EBoolOp op)
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
    VShape res;
    path.transform(res, toSkMatrix(mat->matrix()));
    return res;
  }
  return path;
}

void PaintNode::onDrawAsAlphaMask(Renderer* renderer, sk_sp<SkBlender> blender)
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

void PaintNode::onDrawStyle(
  Renderer*        renderer,
  const VShape&    path,
  const VShape&    mask,
  sk_sp<SkBlender> blender)
{
  d_ptr->onDrawStyleImpl(renderer, path, mask, std::move(blender));
}

VShape PaintNode::asVisualShape(const Transform* mat)
{
  VShape mask;
  mask = makeContourImpl(maskOption(), mat);
  mask.setFillType(childWindingType());
  return mask;
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
  d_ptr->onRevalidateImpl();
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

void PaintNode::paintChildren(Renderer* renderer)
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

  auto paintCall = [&](std::vector<PaintNode*>& nodes)
  {
    if (_->contextSetting.transparencyKnockoutGroup)
    {
      for (const auto& p : nodes)
      {
        p->render(renderer);
      }
    }
    else
    {
      for (const auto& p : nodes)
      {
        p->render(renderer);
      }
    }
  };

  const auto clip = (overflow() == OF_HIDDEN || overflow() == OF_SCROLL);
  {
    SkAutoCanvasRestore acr(canvas, clip);
    if (clip)
    {
      auto boundPath = makeBoundPath();
      boundPath.clip(canvas, SkClipOp::kIntersect);
      // canvas->clipPath(makeBoundPath());
    }
    paintCall(masked);
    paintCall(noneMasked);
  }
}

void PaintNode::paintStyle(Renderer* renderer, const VShape& path, const VShape& outlineMask)
{
}

Bound PaintNode::onDrawFill(
  Renderer*            renderer,
  sk_sp<SkBlender>     blender,
  sk_sp<SkImageFilter> imageFilter,
  const VShape&        path,
  const VShape&        mask)
{
  auto       fillBound = path.bounds();
  FillEffect fillEffect(style().fills, fillBound, imageFilter, blender);
  fillEffect.render(renderer, path);
  return Bound{ fillBound.x(), fillBound.y(), fillBound.width(), fillBound.height() };
}

PaintNode::~PaintNode() = default;

} // namespace VGG::layer
