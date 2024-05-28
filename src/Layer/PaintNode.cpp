/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Layer/Core/VShape.hpp"
#include "Layer/PaintNodeShapeAttributeImpl.hpp"
#include "LayerCache.h"
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

#include <optional>

#define VGG_PAINTNODE_LOG(...) VGG_LOG_DEV(LOG, PaintNode, __VA_ARGS__)

#define VGG_PAINTNODE_DUMP(msg)                                                                    \
  VGG_LAYER_DEBUG_CODE(std::string indent(this->level, '\t');                                      \
                       VGG_PAINTNODE_LOG("{}{}", indent, msg););

namespace
{

#ifdef VGG_LAYER_DEBUG
// std::atomic_int g_paintNodeGlobalID = 0;
// int             genUniqueID()
// {
//   return g_paintNodeGlobalID++;
// }
#endif

} // namespace

namespace VGG::layer
{

class PaintNode__pImpl // NOLINT
{
  VGG_DECL_API(PaintNode);

  friend class MaskBuilder;

public:
  std::string guid{};
  std::string name{};

  EMaskType      maskType{ MT_NONE };
  EMaskShowType  maskShowType{ MST_INVISIBLE };
  EBoolOp        clipOperator{ BO_NONE };
  EOverflow      overflow{ OF_HIDDEN };
  EWindingType   windingRule{ WR_EVEN_ODD };
  ContextSetting contextSetting;
  EObjectType    type;
  bool           visible{ true };
  ContourData    contour;
  ContourOption  maskOption;

  const int uniqueID{ 0 };

  const ERenderTrait renderTrait{ ERenderTraitBits::RT_DEFAULT };

  PaintNode::EventHandler paintNodeEventHandler;

  std::optional<VShape> path;
  Bounds                bounds;

  std::array<float, 4> frameRadius{ 0, 0, 0, 0 };
  float                cornerSmooth{ 0 };

  Ref<StyleItem>            renderNode;
  ShapeAttribute*           shapeItem{ nullptr };
  Ref<TransformAttribute>   transformAttr;
  std::unique_ptr<Accessor> accessor;

  PaintNode__pImpl(
    PaintNode*   api,
    int          uniqueID,
    EObjectType  type,
    ERenderTrait renderTrait,
    bool         initBase)
    : q_ptr(api)
    , type(type)
    , uniqueID(uniqueID)
    , renderTrait(renderTrait)
  {
    transformAttr = TransformAttribute::Make();
    api->observe(transformAttr);

    if (initBase)
    {
      auto [c, d] = StyleItem::MakeRenderNode(
        nullptr,
        api,
        transformAttr,
        [&](VAllocator* alloc, StyleItem* object) -> Ref<GraphicItem>
        {
          auto s = PaintNodeShapeAttributeImpl::Make(alloc, api);
          auto vectorObject = ShapeItem::Make(alloc, s, object);
          shapeItem = s;
          return vectorObject;
        });
      auto acc = std::make_unique<ShapeItemAttibuteAccessor>(*d, shapeItem);
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
    p.get()->d_ptr->worldTransform(mat);
    mat *= q_ptr->transform().matrix();
  }
};

PaintNode::PaintNode(
  VRefCnt*           cnt,
  int                uniqueID,
  const std::string& name,
  EObjectType        type,
  const std::string& guid,
  ERenderTrait       renderTrait,
  bool               initBase)
  : VNode(cnt, EState::INVALIDATE)
  , d_ptr(new PaintNode__pImpl(this, uniqueID, type, renderTrait, initBase))
{
  d_ptr->guid = guid;
  d_ptr->name = name;
#ifdef VGG_LAYER_DEBUG
  dbgInfo = STD_FORMAT("[{} - {}]", name, guid);
#endif
  m_children.reserve(10);
}

int PaintNode::uniqueID() const
{
  return d_ptr->uniqueID;
}

void PaintNode::setContextSettings(const ContextSetting& settings)
{
  VGG_IMPL(PaintNode);
  if (_->contextSetting == settings)
    return;
  _->contextSetting = settings;
  this->invalidate();
}

Transform PaintNode::mapTransform(const PaintNode* node) const
{
  auto findPath = [](const PaintNode* node) -> std::vector<const PaintNode*>
  {
    std::vector<const PaintNode*> path = { node };
    while (node->parent())
    {
      node = node->parent().get();
      path.push_back(node);
    }
    return path;
  };
  auto             path1 = findPath(node);
  auto             path2 = findPath(this);
  const PaintNode* lca = nullptr;
  int              lcaIdx = -1;
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
    auto skm = path1[i]->transform().matrix();
    auto inv = glm::inverse(skm);
    mat = mat * inv;
  }

  for (int i = lcaIdx - 1; i >= 0; i--)
  {
    const auto m = path2[i]->transform().matrix();
    mat = mat * m;
  }
  return Transform(mat);
}

void PaintNode::render(Renderer* renderer)
{
  VGG_IMPL(PaintNode);
  if (!isVisible())
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

      if (_->renderTrait & ERenderTraitBits::RT_RENDER_SELF)
      {
        onPaint(renderer);
      }
      if (_->renderTrait & ERenderTraitBits::RT_RENDER_CHILDREN)
      {
        paintChildren(renderer);
      }
    }
  }
}

void PaintNode::onPaint(Renderer* renderer)
{
  VGG_PAINTNODE_DUMP(STD_FORMAT("PaintNode::onPaint {}", name()));
  d_ptr->renderNode->render(renderer);
}

#ifdef VGG_LAYER_DEBUG
void PaintNode::debug(Renderer* render)
{
  if (!isVisible())
    return;
  auto canvas = render->canvas();
  ASSERT(canvas);
  SkAutoCanvasRestore acr(canvas, true);
  canvas->concat(toSkMatrix(transform().matrix()));
  SkPaint strokePen;
  strokePen.setStyle(SkPaint::kStroke_Style);
  strokePen.setColor(SK_ColorRED);
  strokePen.setStrokeWidth(2);
  canvas->drawRect(toSkRect(bounds().map(transform().inverse())), strokePen);
  if (hoverBounds && d_ptr->renderNode && (d_ptr->renderTrait & ERenderTraitBits::RT_RENDER_SELF))
    d_ptr->renderNode->debug(render);
  for (const auto& e : m_children)
  {
    e->debug(render);
  }
}

std::string PaintNode::dump() const
{
  std::string info;
  const auto& b = d_ptr->accessor->getBackgroundBlurs();
  info = std::format("{}[{}]:\n", name(), guid());
  for (const auto& e : b)
  {
    info += std::string("Background Blur: ") + (e.isEnabled ? "Enabled" : "Disabled");
  }
  return info;
}
#endif

void PaintNode::nodeAt(int x, int y, NodeVisitor visitor, void* userData)
{
  if (!isVisible() || !bounds().valid())
    return;
  if (bounds().contains(x, y))
  {
    auto local = transform().inverse() * glm::vec3(x, y, 1);
    for (auto c = rbegin(); c != rend(); ++c)
    {
      (*c)->nodeAt(local.x, local.y, visitor, userData);
    }
    const NodeAtContext ctx{ .localX = x, .localY = y, .userData = userData };
    visitor(this, &ctx);
  }
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

VShape PaintNode::makeBoundsPath()
{
  return makeFromRectangle(Rectangle{ .bounds = frameBounds(),
                                      .radius = frameRadius(),
                                      .cornerSmoothing = frameCornerSmoothing() });
}

VShape PaintNode::childPolyOperation() const
{
  if (m_children.empty())
  {
    WARN("no child in path %s", name().c_str());
    return VShape();
  }
  if (m_children.size() == 1)
  {
    auto paintNode = m_children.front().get();
    return paintNode->asVisualShape(&paintNode->transform());
  }

  std::vector<std::pair<VShape, EBoolOp>> ct;
  for (auto it = m_children.begin(); it != m_children.end(); ++it)
  {
    auto paintNode = it->get();
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
        [&](const Rectangle& r) { path = makeFromRectangle(r); },
        [&](const Star& s) { path.setStar(s); },
        [&](const Polygon& p) { path.setPolygon(p); },
        [&](const VectorNetwork& p) {},
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
      auto paintNode = it->get();
      auto childMask = paintNode->makeContourImpl(option, &paintNode->transform());
      path.op(childMask, op);
    }
  };

  switch (option.contourType)
  {
    case MCT_FRAMEONLY:
      path = this->makeBoundsPath();
      break;
    case MCT_UNION_WITH_FRAME:
      path = this->makeBoundsPath();
    case MCT_UNION:
      appendPath(path, option, BO_UNION);
      break;
    case MCT_INTERSECT_WITH_FRAME:
      path = this->makeBoundsPath();
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
  if (_->visible == visible)
    return;
  _->visible = visible;
  this->invalidate();
}

bool PaintNode::isVisible() const
{
  return d_ptr->visible;
}

void PaintNode::setFrameRadius(std::array<float, 4> radius)
{
  if (d_ptr->frameRadius == radius)
    return;
  d_ptr->frameRadius = radius;
  if (d_ptr->shapeItem)
    d_ptr->shapeItem->invalidate();
}

std::array<float, 4> PaintNode::frameRadius() const
{
  return d_ptr->frameRadius;
}

void PaintNode::setFrameCornerSmoothing(float smooth)
{
  if (d_ptr->cornerSmooth == smooth)
    return;
  d_ptr->cornerSmooth = smooth;
  if (d_ptr->shapeItem)
    d_ptr->shapeItem->invalidate();
}

float PaintNode::frameCornerSmoothing() const
{
  return d_ptr->cornerSmooth;
}

void PaintNode::setStyle(const Style& style)
{
  VGG_IMPL(PaintNode);
  auto aa = _->accessor.get();
  auto item = aa->styleItem();
  // aa->setFills(style.fills);
  // aa->setBorders(style.borders);
  item->applyFillStyle(style.fills);
  item->applyBorderStyle(style.borders);

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

const Bounds& PaintNode::frameBounds() const
{
  return d_ptr->bounds;
}

void PaintNode::setFrameBounds(const Bounds& bounds)
{
  VGG_IMPL(PaintNode);
  if (_->bounds == bounds)
    return;
  _->bounds = bounds;
  if (_->shapeItem)
    _->shapeItem->invalidate();
}

Bounds PaintNode::onRevalidate()
{
  VGG_IMPL(PaintNode);

  if (!isVisible())
    return Bounds();

  VGG_PAINTNODE_DUMP(STD_FORMAT("{} - {} childs:{}", name(), guid(), m_children.size()));

  for (const auto& e : m_children)
  {
    e->revalidate();
  }

  Bounds bounds = d_ptr->bounds;

  if (overflow() == OF_VISIBLE)
  {
    for (const auto& e : m_children)
    {
      bounds.unionWith(e->bounds());
    }
  }
  _->transformAttr->revalidate();

  if (_->renderTrait & ERenderTraitBits::RT_RENDER_SELF)
  {
    auto currentNodeBounds =
      _->renderNode->revalidate(); // This will trigger the shape attribute get the

    bounds.unionWith(currentNodeBounds);
  }

  return bounds.bounds(transform());
}
const std::string& PaintNode::guid() const
{
  return d_ptr->guid;
}

const std::string& PaintNode::name() const
{
  return d_ptr->name;
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

void PaintNode::paintChildren(Renderer* renderer)
{

  std::vector<PaintNode*> masked;
  std::vector<PaintNode*> noneMasked;
  for (const auto& p : this->m_children)
  {
    auto c = p.get();
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
    for (const auto& p : nodes)
    {
      p->render(renderer);
    }
  };

  const auto clip = (overflow() == OF_HIDDEN || overflow() == OF_SCROLL);
  {
    auto                canvas = renderer->canvas();
    SkAutoCanvasRestore acr(canvas, clip);
    if (clip)
    {
      auto boundsPath = makeBoundsPath();
      boundsPath.clip(canvas, SkClipOp::kIntersect);
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

void PaintNode::onSetShapeAttribute(ShapeAttribute* s)
{
  if (d_ptr->shapeItem == s)
    return;
  d_ptr->shapeItem = s;
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

void PaintNode::addChild(PaintNodePtr node)
{
  m_children.insert(m_children.end(), node);
  node->m_parent = this;
  observe(node);
  this->invalidate();

#ifdef VGG_LAYER_DEBUG
  node->level = level + 1;
#endif
}

void PaintNode::addChild(ChildContainer::const_iterator pos, PaintNodePtr node)
{
  m_children.insert(pos, node);
  node->m_parent = this;
  observe(node);
  this->invalidate();
#ifdef VGG_LAYER_DEBUG
  node->level = level + 1;
#endif
}

PaintNodePtr PaintNode::removeChild(ChildContainer::iterator pos)
{
  if (auto it = m_children.erase(pos); it != m_children.end())
  {
    auto node = *it;
    ASSERT(node);
    unobserve(node);
    this->invalidate();
    return node;
  }
  return nullptr;
}

void PaintNode::removeChild(PaintNodePtr node)
{
  auto it = std::remove(m_children.begin(), m_children.end(), node);
  if (it == m_children.end())
  {
    return;
  }

  m_children.erase(it);
  unobserve(node);
  node->m_parent.release();
  this->invalidate();

#ifdef VGG_LAYER_DEBUG
  node->level = 0;
#endif
}

PaintNode::~PaintNode() = default;

} // namespace VGG::layer
