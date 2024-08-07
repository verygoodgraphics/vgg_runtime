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
#include "LayoutNode.hpp"
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <numeric>
#include "AutoLayout.hpp"
#include "BezierPoint.hpp"
#include "Color.hpp"
#include "DesignModel.hpp"
#include "Domain/Layout/LayoutContext.hpp"
#include "Domain/Model/Element.hpp"
#include "Math.hpp"
#include "Rect.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"

#undef DEBUG
#define DEBUG(msg, ...)

#define VERBOSE DEBUG
#undef VERBOSE
#define VERBOSE(msg, ...)

namespace VGG
{

namespace
{
constexpr auto K_RESIZE_MIN_LENGTH = 1.0;

enum class EBooleanOperation
{
  UNION,
  SUBTRACTION,
  INTERSECION,
  EXCLUSION,
  NONE
};

bool isInvalidLength(Layout::Scalar length)
{
  return length < 0 || doubleNearlyZero(length) || std::isnan(length) || std::isinf(length);
}

} // namespace

std::size_t LayoutNode::treeSize() const
{
  std::size_t count = 1;
  for (const auto& child : children())
    count += child->treeSize();
  return count;
}

std::pair<std::shared_ptr<LayoutNode>, std::string> LayoutNode::hitTest(
  const Layout::Point& point,
  const HitTestHook&   hasEventListener)
{
  // test front child first
  for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
  {
    if ((*it)->pointInside(point))
    {
      if (auto target = (*it)->hitTest(point, hasEventListener); target.first)
      {
        return target;
      }
    }
  }

  if (shouldHandleEvents() && pointInside(point))
  {
    if (!hasEventListener)
    {
      return { shared_from_this(), {} };
    }

    std::vector<std::string> keys{ id(), originalId(), name() };
    if (auto ele = elementNode(); ele && (ele->type() == Domain::Element::EType::SYMBOL_INSTANCE))
    {
      auto s = static_cast<Domain::SymbolInstanceElement*>(ele);
      if (!s->shouldKeepListeners())
        keys.clear();
      keys.insert(keys.begin(), s->masterId());
    }
    for (const auto& key : keys)
    {
      if (hasEventListener(key))
      {
        return { shared_from_this(), key };
      }
    }
  }

  return {};
}

std::shared_ptr<Layout::Internal::AutoLayout> LayoutNode::autoLayout() const
{
  return m_autoLayout;
}

std::shared_ptr<Layout::Internal::AutoLayout> LayoutNode::createAutoLayout()
{
  m_autoLayout.reset(new Layout::Internal::AutoLayout);
  m_autoLayout->view = weak_from_this();

  return m_autoLayout;
}

LayoutNode* LayoutNode::autoLayoutContainer()
{
  return parent();
}

std::shared_ptr<Layout::Internal::AutoLayout> LayoutNode::containerAutoLayout()
{
  if (auto container = autoLayoutContainer())
  {
    return container->autoLayout();
  }

  return nullptr;
}

void LayoutNode::setNeedsLayout()
{
  VERBOSE("LayoutNode::setNeedsLayout: node: %s", id().c_str());
  m_needsLayout = true;
}
void LayoutNode::setContainerNeedsLayout()
{
  autoLayout()->setNeedsLayout();
}

void LayoutNode::layoutIfNeeded(LayoutContext* context)
{
  m_context = context;

  do
  {
    for (auto& child : m_children)
      child->layoutIfNeeded(context);

    updateLayoutSizeInfo();
    updateTextLayoutInfo();

    if (m_needsLayout)
    {
      m_needsLayout = false;

      DEBUG("LayoutNode::layoutIfNeeded: node: %s ", id().c_str());

      // configure container
      configureAutoLayout();

      // configure child items
      for (auto child : m_children)
        child->configureAutoLayout();

      if (m_autoLayout)
        m_autoLayout->applyLayout(true);
    }
  } while (hasNeedsLayoutDescendant());

  m_context = nullptr;
}

Layout::Rect LayoutNode::frame() const
{
  return { origin(), size() };
}

void LayoutNode::setFrame(
  const Layout::Rect& newFrame,
  bool                updateRule,
  bool                useOldFrame,
  bool                duringLayout)
{
  saveChildrendOldFrame(); // save first, before all return

  auto oldFrame = useOldFrame ? m_oldFrame : frame();
  if (oldFrame == newFrame)
  {
    return;
  }

  DEBUG(
    "LayoutNode::setFrame: %s, %s, %s, %f, %f, %f, %f -> %f, %f, %f, %f",
    name().c_str(),
    id().c_str(),
    m_autoLayout->isEnabled() ? "layout" : "scale",
    oldFrame.origin.x,
    oldFrame.origin.y,
    oldFrame.size.width,
    oldFrame.size.height,
    newFrame.origin.x,
    newFrame.origin.y,
    newFrame.size.width,
    newFrame.size.height);

  updateModel(newFrame);

  auto element = elementNode();
  if (!element)
    return;
  if (element->type() == Domain::Element::EType::TEXT)
    m_needsLayoutText = true;

  if (shouldSkip())
  {
    return;
  }

  auto oldSize = oldFrame.size;
  auto newSize = newFrame.size;
  if (newSize == oldSize)
  {
    return;
  }

  if (element->type() == Domain::Element::EType::PATH)
  {
    auto pathElement = static_cast<Domain::PathElement*>(element);
    if (pathElement->children().size() == 1)
    {
      if (const auto c = pathElement->children()[0];
          c && c->type() == Domain::Element::EType::CONTOUR)
      {
        auto contourElement = static_cast<Domain::ContourElement*>(c.get());
        scaleContour(*contourElement, oldFrame, newFrame);
        return;
      }
    }
  }

  if (m_autoLayout && m_autoLayout->isEnabled())
  {
    if (updateRule)
    {
      m_autoLayout->updateSizeRule(size());
    }

    if (m_autoLayout->isContainer())
    {
      resizeChildNodes(oldSize, newSize, true); // resize absolute child
      if (!duringLayout)
      {
        if (m_autoLayout->isAboslutePosition())
          setNeedsLayout();
        else
          setContainerNeedsLayout();
      }
      return;
    }
  }

  resizeChildNodes(oldSize, newSize);
}

Layout::Rect LayoutNode::bounds() const
{
  return modelBounds().makeFromModelRect();
}

Layout::Rect LayoutNode::modelBounds() const
{
  if (auto element = elementNode())
  {
    return element->bounds();
  }
  return {};
}

Layout::Matrix LayoutNode::modelMatrix() const
{
  if (auto element = elementNode())
  {
    return element->matrix();
  }
  return {};
}

Layout::Matrix LayoutNode::modelMatrixWithoutTranslate() const
{
  auto result = modelMatrix();
  result.tx = 0;
  result.ty = 0;
  return result;
}

Layout::Size LayoutNode::size() const
{
  return modelBounds().size;
}

void LayoutNode::dump(std::string indent)
{
  std::cout << indent << id() << ", " << elementNode()->id() << std::endl;

  for (auto& child : m_children)
  {
    child->dump(indent + "  ");
  }
}

uint32_t LayoutNode::backgroundColor()
{
  uint32_t u32Color = 0xFFF5F5F5;

  if (auto element = elementNode())
  {
    if (element->type() == Domain::Element::EType::FRAME)
    {
      auto frameElement = static_cast<Domain::FrameElement*>(element);
      if (auto frameModel = frameElement->object())
      {
        if (frameModel->backgroundColor)
        {
          auto&         colorModel = frameModel->backgroundColor.value();
          Layout::Color color{ colorModel.alpha,
                               colorModel.red,
                               colorModel.green,
                               colorModel.blue };
          u32Color = color.u32();
        }
      }
    }
  }

  return u32Color;
}

void LayoutNode::configureAutoLayout()
{
  if (m_autoLayout)
  {
    m_autoLayout->configure();
  }
}

void LayoutNode::resizeChildNodes(
  const Layout::Size& oldContainerSize,
  const Layout::Size& newContainerSize,
  bool                onlyWhichHasAbsolutePostion)
{
  if (oldContainerSize == Layout::Size{ 0, 0 })
  {
    return;
  }

  if (adjustContentOnResize() == EAdjustContentOnResize::DISABLED)
  {
    return;
  }

  for (auto& child : m_children)
  {
    if (onlyWhichHasAbsolutePostion)
    {
      if (child->autoLayout() && child->autoLayout()->isAboslutePosition())
      {
        child->resize(oldContainerSize, newContainerSize);
      }
    }
    else
    {
      child->resize(oldContainerSize, newContainerSize);
    }
  }
}

bool LayoutNode::isVisible() const
{
  if (auto element = elementNode())
  {
    if (auto pModel = element->object())
    {
      return pModel->visible;
    }
  }
  return true;
}

std::string LayoutNode::vggId() const
{
  return name();
}

std::string LayoutNode::originalId() const
{
  if (auto element = elementNode())
  {
    return element->originalId();
  }

  return {};
}

std::string LayoutNode::name() const
{
  if (auto element = elementNode())
  {
    if (auto pModel = element->model())
    {
      if (pModel->name)
      {
        return pModel->name.value();
      }
    }
  }

  return {};
}

std::string LayoutNode::type() const
{
  if (auto element = elementNode())
  {
    return element->typeString();
  }
  return {};
}

bool LayoutNode::isResizingAroundCenter() const
{
  if (auto element = elementNode())
  {
    if (auto pModel = element->model())
    {
      if (pModel->keepShapeWhenResize)
      {
        return pModel->keepShapeWhenResize.value();
      }
    }
  }

  return false;
}

bool LayoutNode::isVectorNetwork() const
{
  if (auto element = elementNode())
  {
    if (element->type() == Domain::Element::EType::GROUP)
    {
      auto groupElement = static_cast<Domain::GroupElement*>(element);
      if (auto pModel = groupElement->object())
      {
        if (pModel->isVectorNetwork)
        {
          return pModel->isVectorNetwork.value();
        }
      }
    }
  }
  return false;
}

bool LayoutNode::isVectorNetworkDescendant() const
{
  auto p = parent();
  while (p)
  {
    if (p->isVectorNetwork())
    {
      return true;
    }

    p = p->parent();
  }

  return false;
}

LayoutNode* LayoutNode::findDescendantNodeById(const std::string& id)
{
  if (this->id() == id)
  {
    return this;
  }

  for (auto& child : children())
  {
    if (const auto found = child->findDescendantNodeById(id))
    {
      return found;
    }
  }

  return nullptr;
}

void LayoutNode::updateModel(const Layout::Rect& toFrame)
{
  auto element = elementNode();
  if (!element)
  {
    return;
  }

  const auto newFrame = toFrame.makeModelRect();

  // do not update useless frame

  // scale, bounds & matrix
  auto oldBounds = modelBounds();
  auto matrix = modelMatrix();
  if (oldBounds.size.width > 0)
  {
    auto xScale = newFrame.size.width / oldBounds.size.width;
    matrix.tx *= xScale;
  }
  if (oldBounds.size.height > 0)
  {
    auto yScale = newFrame.size.height / oldBounds.size.height;
    matrix.ty *= yScale;
  }
  element->updateBounds(newFrame.size.width, newFrame.size.height);
  element->updateMatrix(matrix.tx, matrix.ty);

  // translate if needed
  auto originAfterScale = modelOrigin();
  if (originAfterScale != newFrame.origin)
  {
    matrix.tx += newFrame.origin.x - originAfterScale.x;
    matrix.ty += newFrame.origin.y - originAfterScale.y;
    element->updateMatrix(matrix.tx, matrix.ty);
  }
  if (auto c = context())
  {
    c->didUpdateBounds(this);
    c->didUpdateMatrix(this);
  }
}

Layout::Point LayoutNode::origin() const
{
  return modelOrigin().makeFromModelPoint();
}

Layout::Point LayoutNode::modelOrigin() const
{
  auto bounds = modelBounds();
  auto matrix = modelMatrix();
  auto [x, y] = bounds.origin;
  auto [a, b, c, d, tx, ty] = matrix;
  auto newX = a * x + c * y + tx;
  auto newY = b * x + d * y + ty;

  Layout::Point relativePosition{ newX, newY };
  if (auto p = parent())
  {
    const auto& parentOrigin = p->modelBounds().origin;
    relativePosition.x -= parentOrigin.x;
    relativePosition.y -= parentOrigin.y;
  }

  return relativePosition;
}

Layout::Point LayoutNode::converPointToAncestor(
  Layout::Point               point,
  std::shared_ptr<LayoutNode> ancestorNode)
{
  if (ancestorNode.get() == this)
  {
    return point;
  }

  auto x = point.x;
  auto y = point.y;

  auto p = parent();
  while (p && p != ancestorNode.get())
  {
    auto origin = p->origin();
    x += origin.x;
    y += origin.y;
    p = p->parent();
  }

  return { x, y };
}

std::shared_ptr<LayoutNode> LayoutNode::scaleTo(
  const Layout::Size& newSize,
  bool                updateRule,
  bool                preservingOrigin)
{
  Layout::Rect newFrame;
  if (preservingOrigin)
  {
    newFrame.origin = frame().origin;
    newFrame.size = newSize;
  }
  else
  {
    newFrame = calculateResizedFrame(newSize);
  }

  saveOldFrame();
  setFrame(newFrame, updateRule, true);
  return autoLayout()->setNeedsLayout();
}

void LayoutNode::saveOldFrame()
{
  m_oldFrame = frame();
  DEBUG(
    "LayoutNode::saveOldFrame: node: %s, %f, %f, %f, %f",
    id().c_str(),
    m_oldFrame.left(),
    m_oldFrame.top(),
    m_oldFrame.width(),
    m_oldFrame.height());
}

void LayoutNode::saveChildrendOldFrame()
{
  for (auto& child : m_children)
  {
    child->saveOldFrame();
  }
}

Layout::Rect LayoutNode::resize(
  const Layout::Size&  oldContainerSize,
  const Layout::Size&  newContainerSize,
  const Layout::Point* parentOrigin,
  bool                 dry)
{
  if (shouldSkip())
  {
    return resizeGroup(oldContainerSize, newContainerSize, parentOrigin);
  }

  auto element = elementNode();
  if (!element)
  {
    return {};
  }
  if (element->type() == Domain::Element::EType::PATH)
  {
    auto pathElement = static_cast<Domain::PathElement*>(element);
    if (pathElement->children().size() == 1)
    {
      if (const auto c = pathElement->children()[0];
          c && c->type() == Domain::Element::EType::CONTOUR)
      {
        auto contourElement = static_cast<Domain::ContourElement*>(c.get());
        return resizeContour(*contourElement, oldContainerSize, newContainerSize, parentOrigin);
      }
    }
  }

  auto       oldFrame = frame();
  const bool keepShapeWithRotation = hasRotation() && isResizingAroundCenter();
  if (keepShapeWithRotation)
  {
    Layout::Point center{ oldFrame.width() / 2, oldFrame.height() / 2 };
    center = center.makeModelPoint().makeTransform(modelMatrix()).makeFromModelPoint();
    oldFrame.setCenter(center);
  }
  saveOldRatio(oldContainerSize, oldFrame, parentOrigin);
  auto [x, w] = resizeH(oldContainerSize, newContainerSize, oldFrame, parentOrigin);
  auto [y, h] = resizeV(oldContainerSize, newContainerSize, oldFrame, parentOrigin);
  Layout::Point newOrigin{ x, y };
  Layout::Rect  newFrame{ newOrigin, { w, h } };
  if (keepShapeWithRotation)
  {
    newOrigin = (newOrigin - newFrame.center())
                  .makeModelPoint()
                  .makeTransform(modelMatrixWithoutTranslate())
                  .makeFromModelPoint() +
                newFrame.center();
    newFrame.origin = newOrigin;
  }

  if (!dry)
    setFrame(newFrame, true, true); // udpate layout rule by constraint

  return newFrame;
}

std::pair<Layout::Scalar, Layout::Scalar> LayoutNode::resizeH(
  const Layout::Size&  oldContainerSize,
  const Layout::Size&  newContainerSize,
  Layout::Rect         oldFrame,
  const Layout::Point* parentOrigin) const
{
  Layout::Scalar x{ 0 }, w{ 0 };

  if (parentOrigin)
  {
    oldFrame = oldFrame.makeOffset(parentOrigin->x, parentOrigin->y);
  }

  DEBUG("resizeH, old container size is %f, %f", oldContainerSize.width, oldContainerSize.height);
  DEBUG(
    "resizeH, old frame is %f, %f,  %f, %f",
    oldFrame.origin.x,
    oldFrame.origin.y,
    oldFrame.size.width,
    oldFrame.size.height);

  const auto rightMargin = *m_rightMargin;

  DEBUG("resizeH, right margin is %f, old frame right is %f", rightMargin, oldFrame.right());

  switch (horizontalResizing())
  {
    case EResizing::FIX_START_FIX_END:
    {
      x = oldFrame.left();
      w = newContainerSize.width - rightMargin - x;
      DEBUG("resizeH, new: x: %f, w: %f", x, w);
    }
    break;

    case EResizing::FIX_START_FIX_SIZE:
    {
      x = oldFrame.left();
      w = oldFrame.width();
    }
    break;

    case EResizing::FIX_START_SCALE:
    {
      x = oldFrame.left();
      const auto wRatio = *m_fixStartWidthRatio;
      DEBUG("resizeH, w ratio is: %f", wRatio);
      w = wRatio * (newContainerSize.width - x);
    }
    break;

    case EResizing::FIX_END_FIX_SIZE:
    {
      w = oldFrame.width();
      x = newContainerSize.width - rightMargin - w;
    }
    break;

    case EResizing::FIX_END_SCALE:
    {
      const auto wRatio = *m_fixEndWidthRatio;
      DEBUG("resizeH, w ratio is: %f", wRatio);
      w = wRatio * (newContainerSize.width - rightMargin);
      x = newContainerSize.width - rightMargin - w;
    }
    break;

    case EResizing::SCALE:
    {
      const auto xScale = newContainerSize.width / oldContainerSize.width;
      x = oldFrame.left() * xScale;
      w = oldFrame.width() * xScale;
    }
    break;

    case EResizing::FIX_CENTER_RATIO_FIX_SIZE:
    {
      const auto centerRatio = oldFrame.centerX() / oldContainerSize.width;
      w = oldFrame.width();
      x = newContainerSize.width * centerRatio - w / 2;
    }
    break;

    case EResizing::FIX_CENTER_OFFSET_FIX_SIZE:
    {
      const auto offset = oldFrame.centerX() - oldContainerSize.width / 2;
      w = oldFrame.width();
      x = newContainerSize.width / 2 + offset - w / 2;
    }
    break;

    default:
      break;
  }

  if (isInvalidLength(w))
  {
    DEBUG("resizeH, w is invalid, %f, set to %f", w, K_RESIZE_MIN_LENGTH);
    w = K_RESIZE_MIN_LENGTH;
  }

  return { x, w };
}

std::pair<Layout::Scalar, Layout::Scalar> LayoutNode::resizeV(
  const Layout::Size&  oldContainerSize,
  const Layout::Size&  newContainerSize,
  Layout::Rect         oldFrame,
  const Layout::Point* parentOrigin) const
{
  Layout::Scalar y{ 0 }, h{ 0 };

  if (parentOrigin)
  {
    oldFrame = oldFrame.makeOffset(parentOrigin->x, parentOrigin->y);
  }

  const auto bottomMargin = *m_bottomMargin;

  switch (verticalResizing())
  {
    case EResizing::FIX_START_FIX_END:
    {
      y = oldFrame.top();
      h = newContainerSize.height - bottomMargin - y;
    }
    break;

    case EResizing::FIX_START_FIX_SIZE:
    {
      y = oldFrame.top();
      h = oldFrame.height();
    }
    break;

    case EResizing::FIX_START_SCALE:
    {
      y = oldFrame.top();
      const auto hRatio = *m_fixStartHeightRatio;
      DEBUG("resizeV, h ratio is: %f", hRatio);
      h = hRatio * (newContainerSize.height - y);
    }
    break;

    case EResizing::FIX_END_FIX_SIZE:
    {
      h = oldFrame.height();
      y = newContainerSize.height - bottomMargin - h;
    }
    break;

    case EResizing::FIX_END_SCALE:
    {
      const auto hRatio = *m_fixEndHeightRatio;
      DEBUG("resizeV, h ratio is: %f", hRatio);
      h = hRatio * (newContainerSize.height - bottomMargin);
      y = newContainerSize.height - bottomMargin - h;
    }
    break;

    case EResizing::SCALE:
    {
      const auto yScale = newContainerSize.height / oldContainerSize.height;
      y = oldFrame.top() * yScale;
      h = oldFrame.height() * yScale;
    }
    break;

    case EResizing::FIX_CENTER_RATIO_FIX_SIZE:
    {
      const auto centerRatio = oldFrame.centerY() / oldContainerSize.height;
      h = oldFrame.height();
      y = newContainerSize.height * centerRatio - h / 2;
    }
    break;

    case EResizing::FIX_CENTER_OFFSET_FIX_SIZE:
    {
      const auto offset = oldFrame.centerY() - oldContainerSize.height / 2;
      h = oldFrame.height();
      y = newContainerSize.height / 2 + offset - h / 2;
    }
    break;

    default:
      break;
  }

  if (isInvalidLength(h))
  {
    DEBUG("resizeV, h is invalid, %f, set to %f", h, K_RESIZE_MIN_LENGTH);
    h = K_RESIZE_MIN_LENGTH;
  }
  return { y, h };
}

LayoutNode::EResizing LayoutNode::horizontalResizing() const
{
  if (isVectorNetworkDescendant())
  {
    return EResizing::SCALE;
  }

  if (auto element = elementNode())
  {
    if (auto pModel = element->object())
    {
      if (pModel->horizontalConstraint)
      {
        return EResizing{ pModel->horizontalConstraint.value() };
      }
    }
  }

  return EResizing::FIX_START_FIX_SIZE;
}

LayoutNode::EResizing LayoutNode::verticalResizing() const
{
  if (isVectorNetworkDescendant())
  {
    return EResizing::SCALE;
  }

  if (auto element = elementNode())
  {
    if (auto pModel = element->object())
    {
      if (pModel->verticalConstraint)
      {
        return EResizing{ pModel->verticalConstraint.value() };
      }
    }
  }
  return EResizing::FIX_START_FIX_SIZE;
}

LayoutNode::EAdjustContentOnResize LayoutNode::adjustContentOnResize() const
{
  if (auto element = elementNode())
  {
    if (auto pModel = element->object())
    {
      if (pModel->resizesContent)
      {
        return EAdjustContentOnResize{ pModel->resizesContent.value() };
      }
    }
  }
  return EAdjustContentOnResize::ENABLED;
}

bool LayoutNode::shouldSkip()
{
  if (m_autoLayout && m_autoLayout->isEnabled())
  {
    return false;
  }

  // parent adjust content is skip group or boolean group && (group || (boolean group))
  if (auto p = parent();
      p && p->adjustContentOnResize() == EAdjustContentOnResize::SKIP_GROUP_OR_BOOLEAN_GROUP)
  {
    if (isVectorNetwork())
    {
      return false;
    }

    if (auto element = elementNode())
    {
      if (element->type() == Domain::Element::EType::GROUP)
      {
        return true;
      }
    }

    return isBooleanGroup();
  }

  return false;
}

bool LayoutNode::isBooleanGroup()
{
  auto element = elementNode();
  if (!element || element->type() != Domain::Element::EType::PATH)
  {
    return false;
  }

  auto pathElement = static_cast<Domain::PathElement*>(element);
  return pathElement->children().size() > 1;
}

Layout::Rect LayoutNode::resizeGroup(
  const Layout::Size&  oldContainerSize,
  const Layout::Size&  newContainerSize,
  const Layout::Point* parentOrigin)
{
  if (m_children.empty())
  {
    return {};
  }

  auto oldOrigin = origin();
  if (parentOrigin)
  {
    oldOrigin += *parentOrigin;
  }

  std::vector<Layout::Rect> childFrames; // relative to ancestor(closest non group container)

  // resize child without group
  for (auto& child : m_children)
  {
    auto childFrame = child->resize(oldContainerSize, newContainerSize, &oldOrigin, true);
    childFrames.push_back(childFrame);
  }

  Layout::Rect newGroupFrame = std::reduce(
    childFrames.begin() + 1,
    childFrames.end(),
    childFrames[0],
    std::mem_fn(&Layout::Rect::makeJoin));
  setFrame(newGroupFrame, false, true);

  auto newOrigin = origin();
  for (std::size_t i = 0; i < childFrames.size(); i++)
  {
    const auto relativeFrame = childFrames[i].makeOffset(-newOrigin.x, -newOrigin.y);
    m_children[i]->setFrame(relativeFrame, false, true);
  }

  // update group frame to fit transformed children
  std::vector<Layout::Rect> childTransformedFrames;
  for (auto& child : m_children)
  {
    childTransformedFrames.push_back(child->transformedFrame());
  }

  if (isBooleanGroup())
  {
    const auto pathElement = static_cast<Domain::PathElement*>(elementNode());
    ASSERT(pathElement->object());
    ASSERT(pathElement->object()->shape);
    auto& subShapes = pathElement->object()->shape->subshapes;

    newGroupFrame = childTransformedFrames[0];
    for (std::size_t i = 1; i < m_children.size(); i++)
    {
      auto&             subShape = subShapes[i];
      EBooleanOperation op{ subShape.booleanOperation };
      switch (op)
      {
        case EBooleanOperation::SUBTRACTION:
          break;

        case EBooleanOperation::INTERSECION:
          newGroupFrame.intersectOrJoin(childTransformedFrames[i]);
          break;

        case EBooleanOperation::UNION:
        case EBooleanOperation::EXCLUSION:
        case EBooleanOperation::NONE:
          newGroupFrame.join(childTransformedFrames[i]);
          break;

        default:
          break;
      }
    }
  }
  else
  {
    newGroupFrame = std::reduce(
      childTransformedFrames.begin() + 1,
      childTransformedFrames.end(),
      childTransformedFrames[0],
      std::mem_fn(&Layout::Rect::makeJoin));
  }
  newGroupFrame = newGroupFrame.makeOffset(newOrigin.x, newOrigin.y);

  setFrame(newGroupFrame, false, false);

  newOrigin = origin();
  for (std::size_t i = 0; i < childFrames.size(); i++)
  {
    const auto relateiveFrame = childFrames[i].makeOffset(-newOrigin.x, -newOrigin.y);
    m_children[i]->setFrame(relateiveFrame, false, true);
  }

  return newGroupFrame;
}

Layout::Rect LayoutNode::transformedFrame() const
{
  auto bounds = modelBounds().makeTransform(modelMatrix(), Layout::Rect::ECoordinateType::MODEL);
  if (auto p = parent())
  {
    const auto& parentOrigin = p->modelBounds().origin;
    bounds = bounds.makeOffset(-parentOrigin.x, -parentOrigin.y);
  }

  return bounds.makeFromModelRect();
}

Layout::Rect LayoutNode::calculateResizedFrame(const Layout::Size& newSize)
{
  const auto         oldFrame = frame();
  const Layout::Rect newFrame{ oldFrame.origin, newSize };
  if (autoLayout()->isFlexOrGridItem())
  {
    // container will layout later, update size only
    return newFrame;
  }

  const auto parentNode = parent();
  if (!parentNode || parentNode->adjustContentOnResize() == EAdjustContentOnResize::DISABLED)
  {
    return newFrame;
  }

  // update by resize flag
  const auto containerSize = parentNode->size();

  // x, w
  Layout::Scalar x{ oldFrame.left() }, w{ newSize.width };
  switch (horizontalResizing())
  {
    case EResizing::FIX_START_FIX_END:
    case EResizing::FIX_START_FIX_SIZE:
    case EResizing::FIX_START_SCALE:
      break;

    case EResizing::FIX_END_FIX_SIZE:
    case EResizing::FIX_END_SCALE:
    {
      const auto rightMargin = containerSize.width - oldFrame.right();
      x = containerSize.width - rightMargin - w;
    }
    break;

    case EResizing::SCALE:
    {
      const auto xScale = w / oldFrame.width();
      x = oldFrame.left() * xScale;
    }
    break;

    case EResizing::FIX_CENTER_RATIO_FIX_SIZE:
    {
      const auto centerRatio = oldFrame.centerX() / containerSize.width;
      x = containerSize.width * centerRatio - w / 2;
    }
    break;

    case EResizing::FIX_CENTER_OFFSET_FIX_SIZE:
    {
      const auto offset = oldFrame.centerX() - containerSize.width / 2;
      x = containerSize.width / 2 + offset - w / 2;
    }
    break;

    default:
      break;
  }

  // y, h
  Layout::Scalar y{ oldFrame.top() }, h{ newSize.height };
  switch (verticalResizing())
  {
    case EResizing::FIX_START_FIX_END:
    case EResizing::FIX_START_FIX_SIZE:
    case EResizing::FIX_START_SCALE:
      break;

    case EResizing::FIX_END_FIX_SIZE:
    case EResizing::FIX_END_SCALE:
    {
      const auto bottomMargin = containerSize.height - oldFrame.bottom();
      y = containerSize.height - bottomMargin - h;
    }
    break;

    case EResizing::SCALE:
    {
      const auto yScale = h / oldFrame.height();
      y = oldFrame.top() * yScale;
    }
    break;

    case EResizing::FIX_CENTER_RATIO_FIX_SIZE:
    {
      const auto centerRatio = oldFrame.centerY() / containerSize.height;
      y = containerSize.height * centerRatio - h / 2;
    }
    break;

    case EResizing::FIX_CENTER_OFFSET_FIX_SIZE:
    {
      const auto offset = oldFrame.centerY() - containerSize.height / 2;
      y = containerSize.height / 2 + offset - h / 2;
    }
    break;

    default:
      break;
  }

  return { { x, y }, { w, h } };
}

Layout::Size LayoutNode::rotatedSize(const Layout::Size& size)
{
  const auto& mat = modelMatrix();
  if (doubleNearlyZero(mat.b) && doubleNearlyZero(mat.c))
  {
    return size;
  }

  Layout::Point p1{ 0, 0 };
  Layout::Point p2{ size.width, 0 };
  Layout::Point p3{ size.width, size.height };
  Layout::Point p4{ 0, size.height };

  p1 = p1.makeTransform(mat);
  p2 = p2.makeTransform(mat);
  p3 = p3.makeTransform(mat);
  p4 = p4.makeTransform(mat);

  const auto& rect = Layout::Rect::makeFromPoints(std::vector<Layout::Point>{ p1, p2, p3, p4 });
  return rect.size;
}

bool LayoutNode::shouldSwapWidthAndHeight()
{
  if (!hasRotation())
    return false;

  const auto& matrix = modelMatrix();
  const auto  radian = std::abs(matrix.decomposeRotateRadian());
  return radian >= M_PI_4 && radian <= M_PI_4 * 3;
}

bool LayoutNode::hasRotation()
{
  const auto& matrix = modelMatrix();
  return !doubleNearlyZero(matrix.b) || !doubleNearlyZero(matrix.c);
}

Layout::Size LayoutNode::swapWidthAndHeightIfNeeded(Layout::Size size)
{
  if (shouldSwapWidthAndHeight())
  {
    std::swap(size.width, size.height);
  }
  return size;
}

Layout::Rect LayoutNode::resizeContour(
  Domain::ContourElement& contourElement,
  const Layout::Size&     oldContainerSize,
  const Layout::Size&     newContainerSize,
  const Layout::Point*    parentOrigin)
{
  DEBUG("resizeContour: begin, name = %s, id = %s", name().c_str(), id().c_str());

  auto pModel = contourElement.dataModel();
  ASSERT(pModel);

  // get model point
  const bool                       isClosed = pModel->closed;
  std::vector<Layout::BezierPoint> oldModelPoints = contourElement.points();
  if (oldModelPoints.empty())
  {
    WARN("LayoutNode::resizeContour: empty points");
    return {};
  }

  // multiply matrix, to layout point
  const auto                       oldModelMatrix = modelMatrix();
  std::vector<Layout::BezierPoint> oldLayoutPoints;
  for (auto tmpPoint : oldModelPoints)
  {
    tmpPoint = tmpPoint.makeTransform(oldModelMatrix).makeFromModelFormat();
    if (parentOrigin)
    {
      // todo, use parent's matrix

      // Make the point's coordinates relative to the group's parent
      tmpPoint = tmpPoint.makeTranslate(parentOrigin->x, parentOrigin->y);
    }
    oldLayoutPoints.push_back(tmpPoint);
    DEBUG(
      "resizeContour: old layout point: %f, %f",
      oldLayoutPoints.back().point.x,
      oldLayoutPoints.back().point.y);
  }

  // resize
  auto oldLayoutFrame = Layout::Rect::makeFromPoints(oldLayoutPoints, isClosed);
  saveOldRatio(oldContainerSize, oldLayoutFrame, nullptr);
  auto [x, w] = resizeH(
    oldContainerSize,
    newContainerSize,
    oldLayoutFrame,
    nullptr); // Pass nullptr because oldLayoutPoints has been translated by parentOrigin
  auto [y, h] = resizeV(oldContainerSize, newContainerSize, oldLayoutFrame, nullptr);
  Layout::Rect newLayoutFrame{ { x, y }, { w, h } };
  DEBUG(
    "resizeContour: old layout frame: %f, %f, %f, %f",
    oldLayoutFrame.left(),
    oldLayoutFrame.top(),
    oldLayoutFrame.width(),
    oldLayoutFrame.height());
  DEBUG(
    "resizeContour: new layout frame: %f, %f, %f, %f",
    newLayoutFrame.left(),
    newLayoutFrame.top(),
    newLayoutFrame.width(),
    newLayoutFrame.height());

  // new point
  std::vector<Layout::BezierPoint> newLayoutPoints;
  for (const auto& oldLayoutPoint : oldLayoutPoints)
  {
    newLayoutPoints.push_back(oldLayoutPoint.makeScale(oldLayoutFrame, newLayoutFrame));
    DEBUG(
      "resizeContour: new layout point, %lu: %f, %f",
      newLayoutPoints.size() - 1,
      newLayoutPoints.back().point.x,
      newLayoutPoints.back().point.y);
  }

  const float modelRadian = oldModelMatrix.decomposeRotateRadian();
  // In the model coordinate system, the y-axis direction is upward
  // In the layout coordinate system, the y-axis direction is downward
  const float layoutRadian = modelRadian * Layout::FLIP_Y_FACTOR;

  std::vector<Layout::BezierPoint> reverseRotatedLayoutPoints;
  const auto reverseRotateTransform = Layout::Matrix::makeRotate(-layoutRadian);
  for (const auto& newLayoutPoint : newLayoutPoints)
  {
    reverseRotatedLayoutPoints.push_back(newLayoutPoint.makeTransform(reverseRotateTransform));

    DEBUG(
      "resizeContour: reverse rotated point, %f, %f",
      reverseRotatedLayoutPoints.back().point.x,
      reverseRotatedLayoutPoints.back().point.y);
  }
  const auto reverseRotatedFrame =
    Layout::Rect::makeFromPoints(reverseRotatedLayoutPoints, isClosed);
  DEBUG(
    "resizeContour: reverse rotated frame, %f, %f, %f, %f",
    reverseRotatedFrame.left(),
    reverseRotatedFrame.top(),
    reverseRotatedFrame.width(),
    reverseRotatedFrame.height());

  // calculate new model point
  std::vector<Layout::BezierPoint> newModelPoints;
  for (const auto& reverseRotatedPoint : reverseRotatedLayoutPoints)
  {
    auto newModelPoint =
      reverseRotatedPoint.makeTranslate(-reverseRotatedFrame.left(), -reverseRotatedFrame.top())
        .makeModelFormat();
    DEBUG("resizeContour: new model point, %f, %f", newModelPoint.point.x, newModelPoint.point.y);
    newModelPoints.push_back(newModelPoint);
  }

  // calculate new matrix
  auto          relativeFirstLayoutPoint = newModelPoints[0].makeFromModelFormat();
  Layout::Point relativeFirstPoint{
    relativeFirstLayoutPoint.point.x,
    relativeFirstLayoutPoint.point.y
  }; // rotate anchor is top left point
  const auto rotateTransform = Layout::Matrix::makeRotate(layoutRadian);
  auto       rotatedRelativeFirstPoint = relativeFirstPoint.makeTransform(rotateTransform);
  auto       xOffset = newLayoutPoints[0].point.x - rotatedRelativeFirstPoint.x;
  auto       yOffset = newLayoutPoints[0].point.y - rotatedRelativeFirstPoint.y;

  const Layout::Point layoutTopLeft{ xOffset, yOffset };
  const auto          newModelTopLeft = layoutTopLeft.makeModelPoint();

  const Layout::Rect newModelFrame{ newModelTopLeft, reverseRotatedFrame.size };
  auto newMatrix = Layout::Matrix::make(newModelTopLeft.x, newModelTopLeft.y, modelRadian);

  updatePathNodeModel(newModelFrame.makeModelRect(), newMatrix, newModelPoints);

  DEBUG(
    "resizeContour: end, name = %s, id = %s, new frame, %f, %f, %f, %f",
    name().c_str(),
    id().c_str(),
    layoutTopLeft.x,
    layoutTopLeft.y,
    reverseRotatedFrame.width(),
    reverseRotatedFrame.height());

  return newModelFrame.makeFromModelRect();
}

void LayoutNode::scaleContour(
  Domain::ContourElement& contourElement,
  const Layout::Rect&     oldFrame,
  const Layout::Rect&     newFrame)
{
  DEBUG("LayoutNode::scaleContour, id=%s", id().c_str());

  const auto oldSize = oldFrame.size;
  const auto newSize = newFrame.size;

  const auto xScaleFactor = doubleNearlyZero(oldSize.width) ? 0 : newSize.width / oldSize.width;
  const auto yScaleFactor = doubleNearlyZero(oldSize.height) ? 0 : newSize.height / oldSize.height;

  const auto&                      points = contourElement.points();
  std::vector<Layout::BezierPoint> newModelPoints;
  for (auto point : points)
  {
    point.scale(xScaleFactor, yScaleFactor);
    newModelPoints.push_back(point);
  }
  contourElement.updatePoints(newModelPoints);
}

void LayoutNode::updatePathNodeModel(
  const Layout::Rect&                     newFrame,
  const Layout::Matrix&                   matrix,
  const std::vector<Layout::BezierPoint>& newPoints)
{
  auto element = elementNode();
  if (!element || element->type() != Domain::Element::EType::PATH)
  {
    return;
  }

  auto pathElement = static_cast<Domain::PathElement*>(element);
  if (!pathElement)
  {
    return;
  }

  // do not update useless frame
  pathElement->updateBounds(newFrame.width(), newFrame.height());
  pathElement->updateMatrix({ matrix.a, matrix.b, matrix.c, matrix.d, matrix.tx, matrix.ty });
  if (auto c = context())
  {
    c->didUpdateBounds(this);
    c->didUpdateMatrix(this);
  }

  for (auto& child : pathElement->children())
  {
    if (child && child->type() == Domain::Element::EType::CONTOUR)
    {
      auto contourElement = static_cast<Domain::ContourElement*>(child.get());
      contourElement->updatePoints(newPoints);
      if (auto c = context())
        c->didUpdateContourPoints(this);
    }
  }
}

std::vector<std::shared_ptr<LayoutNode>> LayoutNode::removeAllChildren()
{
  autoLayout()->removeSubtree();
  return std::move(m_children);
}

void LayoutNode::detachChildrenFromFlexNodeTree()
{
  DEBUG(
    "LayoutNode::detachChildrenFromFlexNodeTree: name = %s, id = %s",
    name().c_str(),
    id().c_str());

  // Pay attention to the deletion order, the last index must be deleted first, or the index
  // will change
  for (auto it = children().rbegin(); it != children().rend(); ++it)
  {
    (*it)->autoLayout()->takeFlexNodeFromTree();
  }
}

LayoutNode* LayoutNode::closestCommonAncestor(LayoutNode* node)
{
  auto p = this;
  while (p)
  {
    if (p->isAncestorOf(node))
    {
      return p;
    }

    p = p->parent();
  }

  return nullptr;
}

bool LayoutNode::isAncestorOf(const LayoutNode* node)
{
  auto p = node;
  while (p)
  {
    if (p == this)
    {
      return true;
    }

    p = p->parent();
  }

  return false;
}

void LayoutNode::saveOldRatio(
  const Layout::Size&  oldContainerSize,
  Layout::Rect         oldFrame,
  const Layout::Point* parentOrigin)
{
  if (parentOrigin)
  {
    oldFrame = oldFrame.makeOffset(parentOrigin->x, parentOrigin->y);
  }

  if (!m_rightMargin)
  {
    m_rightMargin = oldContainerSize.width - oldFrame.right();
  }
  switch (horizontalResizing())
  {
    case EResizing::FIX_START_SCALE:
    {
      if (!m_fixStartWidthRatio)
      {
        m_fixStartWidthRatio = oldFrame.width() / (oldContainerSize.width - oldFrame.left());
        DEBUG("FIX_START_SCALE, w ratio is: %f", *m_fixStartWidthRatio);
      }
    }
    break;

    case EResizing::FIX_END_SCALE:
    {
      if (!m_fixEndWidthRatio)
      {
        m_fixEndWidthRatio = oldFrame.width() / (oldContainerSize.width - *m_rightMargin);
        DEBUG("FIX_END_SCALE, w ratio is: %f", *m_fixEndWidthRatio);
      }
    }
    break;

    default:
      break;
  }

  if (!m_bottomMargin)
  {
    m_bottomMargin = oldContainerSize.height - oldFrame.bottom();
  }
  switch (verticalResizing())
  {
    case EResizing::FIX_START_SCALE:
    {
      if (!m_fixStartHeightRatio)
      {
        m_fixStartHeightRatio = oldFrame.height() / (oldContainerSize.height - oldFrame.top());
        DEBUG("FIX_START_SCALE, h ratio is: %f", *m_fixStartHeightRatio);
      }
    }
    break;

    case EResizing::FIX_END_SCALE:
    {
      if (!m_fixEndHeightRatio)
      {
        m_fixEndHeightRatio = oldFrame.height() / (oldContainerSize.height - *m_bottomMargin);
        DEBUG("FIX_END_SCALE, h ratio is: %f", *m_fixEndHeightRatio);
      }
    }
    break;

    default:
      break;
  }
}

std::shared_ptr<LayoutNode> StateTree::srcNode() const
{
  if (!m_srcNode.lock()) // Node may become invalid and rebuilt during expanding instance
  {
    auto page = m_pageNode.lock();
    ASSERT(page);

    if (auto p = page->findDescendantNodeById(m_srcNodeId))
      const_cast<StateTree*>(this)->m_srcNode = p->shared_from_this();
  }

  return m_srcNode.lock();
}

void StateTree::setSrcNode(std::shared_ptr<LayoutNode> srcNode)
{
  ASSERT(srcNode);
  m_srcNode = srcNode;
  m_srcNodeId = srcNode->id();
}

const std::string& LayoutNode::LayoutNode::id() const
{
  if (!m_hasIdCache)
  {
    if (const auto element = elementNode())
    {
      m_id = element->id();
      m_hasIdCache = true;
    }
  }

  return m_id;
}

void LayoutNode::invalidateIdCache()
{
  m_id.clear();
  m_hasIdCache = false;
  for (auto& child : m_children)
  {
    child->invalidateIdCache();
  }
}

void LayoutNode::updateLayoutSizeInfo()
{
  if (!m_autoLayout)
    return;

  bool hasUnkownWidthChild{ false };
  bool hasUnknownHeightChild{ false };
  bool hasFixedWidthChild{ false };
  bool hasFixedHeightChild{ false };
  for (auto& child : m_children)
  {
    bool childIsFixedWidth{ false };
    bool childIsFixedHeight{ false };
    if (child->m_autoLayout)
      child->m_autoLayout->isFixedSize(childIsFixedWidth, childIsFixedHeight);

    hasUnkownWidthChild |= !childIsFixedWidth;
    hasUnknownHeightChild |= !childIsFixedHeight;

    hasFixedWidthChild |= childIsFixedWidth;
    hasFixedHeightChild |= childIsFixedHeight;
  }

  m_autoLayout->setHasUnknownWidthChild(hasUnkownWidthChild);
  m_autoLayout->setHasUnknownHeightChild(hasUnknownHeightChild);
  m_autoLayout->setHasFixedWidthChild(hasFixedWidthChild);
  m_autoLayout->setHasFixedHeightChild(hasFixedHeightChild);
}

bool LayoutNode::hasNeedsLayoutDescendant() const
{
  if (needsLayout())
    return true;

  for (auto& child : m_children)
    if (child->hasNeedsLayoutDescendant())
      return true;

  return false;
}

LayoutContext* LayoutNode::context()
{
  if (m_context)
    return m_context;
  if (auto p = parent())
    return p->context();
  return nullptr;
}

void LayoutNode::updateTextLayoutInfo()
{
  if (!m_needsLayoutText)
    return;

  auto element = elementNode();
  if (!element)
    return;
  if (element->type() != Domain::Element::EType::TEXT)
    return;

  if (auto c = context(); c && m_autoLayout && m_autoLayout->isEnabled())
  {
    const auto maybePaintSize = c->nodeSize(this);
    if (maybePaintSize)
    {
      if (const auto& paintSize = *maybePaintSize; paintSize != size())
      {
        m_autoLayout->updateSizeRule(paintSize);
        m_autoLayout->setNeedsLayout();
      }

      m_needsLayoutText = false;
    }
    else
    {
      // layout if needed after paint node ready
      m_needsLayoutText = true;
    }
  }
}

} // namespace VGG
