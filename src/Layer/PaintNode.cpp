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
#include "Guard.hpp"
#include "LayerCache.h"
#include "Settings.hpp"
#include "SkSL.hpp"
#include "Effects.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "ShapeItem.hpp"
#include "StyleItem.hpp"

#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/TreeNode.hpp"

#include <optional>

namespace
{
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
} // namespace

namespace VGG::layer
{

class PaintNode__pImpl // NOLINT
{
  VGG_DECL_API(PaintNode);

  friend class MaskBuilder;

public:
  std::string guid{};

  EMaskType      maskType{ MT_NONE };
  EMaskShowType  maskShowType{ MST_INVISIBLE };
  EBoolOp        clipOperator{ BO_NONE };
  EOverflow      overflow{ OF_HIDDEN };
  EWindingType   windingRule{ WR_EVEN_ODD };
  ContextSetting contextSetting;
  EObjectType    type;
  bool           visible{ true };
  ContourData    contour;
  PaintOption    paintOption;
  ContourOption  maskOption;

  PaintNode::EventHandler paintNodeEventHandler;

  std::optional<VShape> path;
  bool                  renderable{ false };
  Bounds                bound;

  std::array<float, 4> frameRadius{ 0, 0, 0, 0 };
  float                cornerSmooth{ 0 };

  Ref<StyleItem>            renderNode;
  Ref<TransformAttribute>   transformAttr;
  std::unique_ptr<Accessor> accessor;

  PaintNode__pImpl(PaintNode* api, EObjectType type, bool initBase)
    : q_ptr(api)
    , type(type)
  {
    transformAttr = TransformAttribute::Make();
    api->observe(transformAttr);

    if (initBase)
    {
      Ref<ShapeAttribute> shape;
      auto [c, d] = StyleItem::MakeRenderNode(
        nullptr,
        api,
        transformAttr,
        [&](VAllocator* alloc, ObjectAttribute* object) -> Ref<GraphicItem>
        {
          shape = ShapeAttribute::Make(alloc, api);
          auto vectorObject = ShapeItem::Make(alloc, shape, object);
          return vectorObject;
        });
      auto acc = std::make_unique<ShapeItemAttibuteAccessor>(*d, shape.get());
      accessor = std::move(acc);
      renderNode = std::move(c);
      api->observe(renderNode);
    }
  }

  void worldTransform(glm::mat3& mat)
  {
    auto p = q_ptr->parent();
    if (!p)
    {
      mat *= q_ptr->transform().matrix();
      return;
    }
    static_cast<PaintNode*>(p.get())->d_ptr->worldTransform(mat);
    mat *= q_ptr->transform().matrix();
  }
};

PaintNode::PaintNode(
  VRefCnt*           cnt,
  const std::string& name,
  EObjectType        type,
  const std::string& guid,
  bool               initBase)
  : TreeNode(cnt, name)
  , d_ptr(new PaintNode__pImpl(this, type, initBase))
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
        canvas->drawRect(toSkRect(bounds()), strokePen);
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
  d_ptr->renderNode->render(renderer);
}

bool PaintNode::nodeAt(int x, int y, NodeVisitor visitor)
{
  if (!isVisible())
    return true;
  auto local = transform().inverse() * glm::vec3(x, y, 1);
  if (bounds().contains(local.x, local.y))
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
  _->accessor->setShapeMask(std::move(masks));
}

void PaintNode::setAlphaMaskBy(std::vector<AlphaMask> masks)
{
  VGG_IMPL(PaintNode);
  _->accessor->setAlphaMask(std::move(masks));
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
  auto aa = _->accessor.get();
  aa->setFills(style.fills);
  aa->setBorders(style.borders);
  aa->setInnerShadows(style.innerShadow);
  aa->setDropShadows(style.dropShadow);
  aa->setLayerBlurs(style.layerEffects);
  aa->setBackgroundBlurs(style.backgroundEffects);
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

const Bounds& PaintNode::frameBound() const
{
  return d_ptr->bound;
}

void PaintNode::setFrameBounds(const Bounds& bound)
{
  VGG_IMPL(PaintNode);
  _->bound = bound;
  invalidate();
}

Bounds PaintNode::onRevalidate()
{
  VGG_IMPL(PaintNode);
  Bounds newBound;
  for (const auto& e : m_firstChild)
  {
    newBound.unionWith(e->revalidate());
  }
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

void PaintNode::onSetAccessor(std::unique_ptr<Accessor> acc)
{
  d_ptr->accessor = std::move(acc);
}
void PaintNode::onSetStyleItem(Ref<StyleItem> item)
{
  if (d_ptr->renderNode == item)
    return;
  d_ptr->renderNode = std::move(item);
}

StyleItem* PaintNode::styleItem()
{
  return d_ptr->renderNode.get();
}

void PaintNode::dispatchEvent(void* event)
{
  VGG_IMPL(PaintNode);
  if (_->paintNodeEventHandler)
  {
    _->paintNodeEventHandler(static_cast<ShapeItemAttibuteAccessor*>(attributeAccessor()), event);
  }
}

PaintNode::~PaintNode() = default;

} // namespace VGG::layer
