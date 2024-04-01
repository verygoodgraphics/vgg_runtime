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
#include "Layer/AttributeAccessor.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Guard.hpp"
#include "Layer/LayerCache.h"
#include "Layer/Settings.hpp"
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

inline SkColor nodeType2Color(EObjectType type)
{
  switch (type)
  {
    case EObjectType::VGG_PATH:
      return SK_ColorRED;
    case EObjectType::VGG_IMAGE:
      return SK_ColorRED;
    case EObjectType::VGG_GROUP:
      return SK_ColorRED;
    case EObjectType::VGG_TEXT:
      return SK_ColorRED;
    case EObjectType::VGG_ARTBOARD:
      return SK_ColorRED;
    case EObjectType::VGG_LAYER:
      return SK_ColorRED;
    case EObjectType::VGG_MASTER:
      return SK_ColorRED;
    case EObjectType::VGG_CONTOUR:
      return SK_ColorYELLOW;
    default:
      return SK_ColorRED;
  }
}

namespace VGG::layer
{

PaintNode::PaintNode(
  VRefCnt*           cnt,
  const std::string& name,
  EObjectType        type,
  const std::string& guid,
  bool               legacyCode,
  bool               initVectorRenderNode)
  : TreeNode(cnt, name)
  , d_ptr(new PaintNode__pImpl(this, type, legacyCode, initVectorRenderNode))
{
  auto renderObject = d_ptr->guid = guid;
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
    auto skm = static_cast<const PaintNode*>(path1[i])->transform().matrix();
    auto inv = glm::inverse(skm);
    mat = mat * inv;
  }

  for (int i = lcaIdx - 1; i >= 0; i--)
  {
    const auto m = static_cast<const PaintNode*>(path2[i])->transform().matrix();
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
      canvas->concat(toSkMatrix(transform().matrix()));
      if (getDebugBoundEnable())
      {
        SkPaint strokePen;
        strokePen.setStyle(SkPaint::kStroke_Style);
        SkColor color = nodeType2Color(VGG_PATH);
        strokePen.setColor(color);
        strokePen.setStrokeWidth(2);
        canvas->drawRect(toSkRect(bound()), strokePen);
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
  if (_->renderable)
  {
    this->onPaint(renderer);
  }
}

void PaintNode::onPaint(Renderer* renderer)
{
  VGG_IMPL(PaintNode);

  if (_->legacyCode)
  {
    auto dropbackFilter = _->backgroundBlurImageFilter();
    auto layerFilter = _->blurImageFilter();

    const auto newLayer = dropbackFilter || layerFilter || !_->alphaMaskBy.empty();
    ASSERT(_->path);
    auto&  path = *_->path;
    VShape shapeMask;
    if (!_->maskedBy.empty())
    {
      auto iter = ShapeMaskIterator(_->maskedBy);
      shapeMask = MaskBuilder::makeShapeMask(this, *getMaskMap(), iter, toSkRect(frameBound()), 0);
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
        auto     alphaMaskIter = AlphaMaskIterator(_->alphaMaskBy);
        SkMatrix resetOffset = SkMatrix::Translate(
          layerBound.x(),
          layerBound.y()); // note that rasterized shader is located in the origin, we need a matrix
                           // to reset the offset
        layerFilter = MaskBuilder::makeAlphaMaskWith(
          layerFilter,
          this,
          *getMaskMap(),
          alphaMaskIter,
          layerBound,
          &resetOffset);
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
      if (getDebugBoundEnable())
      {
        lp.setColor(SK_ColorBLUE);
        lp.setStrokeWidth(5);
        renderer->canvas()->drawRect(layerBound, lp);
        lp.setColor(SK_ColorCYAN);
        lp.setStrokeWidth(4);
        renderer->canvas()->drawRect(_->dropShadowEffects->bounds(), lp);
        lp.setColor(SK_ColorGREEN);
        lp.setStrokeWidth(3);
        renderer->canvas()->drawRect(_->styleDisplayList->bounds(), lp);
        lp.setStrokeWidth(2);
        lp.setColor(SK_ColorYELLOW);
        renderer->canvas()->drawRect(objectBound, lp);
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
  else
  {
    d_ptr->renderNode->render(renderer);
  }
}

bool PaintNode::nodeAt(int x, int y, NodeVisitor visitor)
{
  if (!isVisible())
    return true;
  auto local = transform().inverse() * glm::vec3(x, y, 1);
  if (bound().contains(local.x, local.y))
  {
    for (auto c = rbegin(); c != rend(); ++c)
    {
      auto n = static_cast<PaintNode*>(c->get());
      if (!n->nodeAt(local.x, local.y, visitor))
      {
        return false;
      }
    }
    return visitor(this);
  }
  return true;
}

void PaintNode::setMaskBy(std::vector<std::string> masks)
{
  VGG_IMPL(PaintNode);
  _->maskedBy = std::move(masks);
  if (!_->legacyCode)
  {
    _->accessor->setShapeMask(_->maskedBy);
  }
}

void PaintNode::setAlphaMaskBy(std::vector<AlphaMask> masks)
{
  VGG_IMPL(PaintNode);
  _->alphaMaskBy = std::move(masks);
  if (!_->legacyCode)
    _->accessor->setAlphaMask(_->alphaMaskBy);
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
    makeShape(d_ptr->frameRadius, skRect, d_ptr->cornerSmooth));
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

void PaintNode::setFrameRadius(std::array<float, 4> radius)
{
  d_ptr->frameRadius = radius;
}

std::array<float, 4> PaintNode::frameRadius() const
{
  return d_ptr->frameRadius;
}

void PaintNode::setFrameCornerSmoothing(float smooth)
{
  d_ptr->cornerSmooth = smooth;
}

float PaintNode::frameCornerSmoothing() const
{
  return d_ptr->cornerSmooth;
}

void PaintNode::setStyle(const Style& style)
{
  VGG_IMPL(PaintNode);
  _->style = style;
  if (!_->legacyCode)
  {
    auto aa = _->accessor.get();
    aa->setFill(style.fills);
    aa->setBorder(style.borders);
    aa->setInnerShadow(style.innerShadow);
    aa->setDropShadow(style.dropShadow);
    aa->setLayerBlur(style.layerEffects);
    aa->setBackgroundBlur(style.backgroundEffects);
  }
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
  _->transformAttr->setTransform(transform);
}

const Transform& PaintNode::transform() const
{
  return d_ptr->transformAttr->getTransform();
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
  VGG_IMPL(PaintNode);
  Bound newBound;
  for (const auto& e : m_firstChild)
  {
    newBound.unionWith(e->revalidate());
  }

  if (_->legacyCode)
  {
    if (
      _->paintOption.paintStrategy == EPaintStrategy::PS_SELFONLY ||
      _->paintOption.paintStrategy == EPaintStrategy::PS_RECURSIVELY)
    {
      if (!_->path)
      {
        _->path = VShape(asVisualShape(0));
      }
      _->renderable = true;
    }
    else
    {
      _->renderable = false;
    }
    newBound.unionWith(d_ptr->bound);
    // return newBound;
    return d_ptr->bound;
  }
  else
  {
    _->transformAttr->revalidate();
    if (
      _->paintOption.paintStrategy == EPaintStrategy::PS_SELFONLY ||
      _->paintOption.paintStrategy == EPaintStrategy::PS_RECURSIVELY)
    {
      auto currentNodeBound =
        _->renderNode->revalidate(); // This will trigger the shape attribute get the

      newBound.unionWith(currentNodeBound);
      _->renderable = true;
    }
    else
    {
      _->renderable = false;
    }
    // shape from the current node by the passed node
    // return newBound;
    return newBound;
  }
}
const std::string& PaintNode::guid() const
{
  return d_ptr->guid;
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
    auto                canvas = renderer->canvas();
    SkAutoCanvasRestore acr(canvas, clip);
    if (clip)
    {
      auto boundPath = makeBoundPath();
      boundPath.clip(canvas, SkClipOp::kIntersect);
    }
    paintCall(masked);
    paintCall(noneMasked);
  }
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

Accessor* PaintNode::attributeAccessor()
{
  ASSERT(d_ptr->accessor);
  return d_ptr->accessor.get();
}
TransformAttribute* PaintNode::transformAttribute()
{
  return d_ptr->transformAttr;
}

void PaintNode::installPaintNodeEventHandler(EventHandler handler)
{
  d_ptr->paintNodeEventHandler = std::move(handler);
}

void PaintNode::dispatchEvent(void* event)
{
  VGG_IMPL(PaintNode);
  if (_->paintNodeEventHandler)
  {
    _->paintNodeEventHandler(
      static_cast<VectorObjectAttibuteAccessor*>(attributeAccessor()),
      event);
  }
}

PaintNode::~PaintNode() = default;

} // namespace VGG::layer
