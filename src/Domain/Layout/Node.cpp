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
#include "Node.hpp"

#include "AutoLayout.hpp"
#include "BezierPoint.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Rect.hpp"

#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"

#include <functional>
#include <iostream>
#include <numeric>

#undef DEBUG
#define DEBUG(msg, ...)

namespace
{
enum class EBooleanOperation
{
  UNION,
  SUBTRACTION,
  INTERSECION,
  EXCLUSION,
  NONE
};
} // namespace

namespace VGG
{

std::shared_ptr<LayoutNode> LayoutNode::hitTest(
  const Layout::Point& point,
  const HitTestHook&   hasEventListener)
{
  // test front child first
  for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
  {
    if ((*it)->pointInside(point))
    {
      if (auto targetNode = (*it)->hitTest(point, hasEventListener))
      {
        return targetNode;
      }
    }
  }

  if (pointInside(point))
  {
    if (hasEventListener(vggId()))
    {
      return shared_from_this();
    }
    if (hasEventListener(m_path))
    {
      return shared_from_this();
    }
  }

  return nullptr;
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

std::shared_ptr<LayoutNode> LayoutNode::autoLayoutContainer()
{
  return m_parent.lock();
}

void LayoutNode::setNeedLayout()
{
  m_needsLayout = true;
}

void LayoutNode::layoutIfNeeded()
{
  for (auto child : m_children)
  {
    child->layoutIfNeeded();
  }

  if (m_needsLayout)
  {
    m_needsLayout = false;

    DEBUG("LayoutNode::layoutIfNeeded: node: %s, %s", id().c_str(), path().c_str());

    // configure container
    configureAutoLayout();

    // configure child items
    for (auto child : m_children)
    {
      child->configureAutoLayout();
    }

    if (m_autoLayout)
    {
      m_autoLayout->applyLayout(true);
    }
  }
}

Layout::Rect LayoutNode::frame() const
{
  return { origin(), size() };
}

void LayoutNode::setFrame(const Layout::Rect& newFrame, bool updateRule, bool useOldFrame)
{
  saveChildrendOldFrame(); // save first, before all return

  auto oldFrame = useOldFrame ? m_oldFrame : frame();
  if (oldFrame == newFrame)
  {
    return;
  }

  DEBUG(
    "LayoutNode::setFrame: %s, %s, %s, %s, %f, %f, %f, %f -> %f, %f, %f, %f",
    name().c_str(),
    id().c_str(),
    path().c_str(),
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

  if (auto json = model(); Layout::isContourPathNode(*json))
  {
    scaleContour(oldFrame, newFrame);
    return;
  }

  if (m_autoLayout && m_autoLayout->isEnabled())
  {
    if (updateRule)
    {
      m_autoLayout->updateSizeRule();
    }

    if (m_autoLayout->isContainer())
    {
      resizeChildNodes(oldSize, newSize, true); // resize absolute child
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
  if (auto json = model())
  {
    return (*json)[K_BOUNDS].get<Layout::Rect>();
  }
  return {};
}

Layout::Matrix LayoutNode::modelMatrix() const
{
  if (auto json = model())
  {
    return (*json)[K_MATRIX].get<Layout::Matrix>();
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

void LayoutNode::setViewModel(JsonDocumentPtr viewModel)
{
  m_viewModel = viewModel;
}

void LayoutNode::dump(std::string indent)
{
  std::cout << indent << id() << ", " << path() << std::endl;

  for (auto& child : m_children)
  {
    child->dump(indent + "  ");
  }
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
  return getValue(K_VISIBLE, true);
}

std::string LayoutNode::vggId() const
{
  return name();
}

std::string LayoutNode::id() const
{
  return getValue(K_ID, std::string{});
}

std::string LayoutNode::name() const
{
  return getValue(K_NAME, std::string{});
}

bool LayoutNode::isResizingAroundCenter() const
{
  return getValue(K_KEEP_SHAPE_WHEN_RESIZE, false);
}

bool LayoutNode::isVectorNetwork() const
{
  return getValue(K_IS_VECTOR_NETWORK, false);
}

bool LayoutNode::isVectorNetworkDescendant() const
{
  auto parent = m_parent.lock();
  while (parent)
  {
    if (parent->isVectorNetwork())
    {
      return true;
    }

    parent = parent->m_parent.lock();
  }

  return false;
}

std::shared_ptr<LayoutNode> LayoutNode::findDescendantNodeById(const std::string& id)
{
  if (this->id() == id)
  {
    return shared_from_this();
  }

  for (auto child : children())
  {
    if (auto found = child->findDescendantNodeById(id))
    {
      return found;
    }
  }

  return nullptr;
}

void LayoutNode::updateModel(const Layout::Rect& toFrame)
{
  auto viewModel = m_viewModel.lock();
  if (!viewModel)
  {
    return;
  }

  const auto newFrame = toFrame.makeModelRect();

  nlohmann::json::json_pointer path{ m_path };
  const auto&                  objectJson = viewModel->content()[path];

  // do not update useless frame

  // scale, bounds & matrix
  auto boundsJson = objectJson[K_BOUNDS];
  auto oldBounds = modelBounds();
  auto matrix = modelMatrix();
  if (oldBounds.size.width > 0)
  {
    auto xScale = newFrame.size.width / oldBounds.size.width;
    boundsJson[K_X] = oldBounds.origin.x * xScale;
    matrix.tx *= xScale;
  }
  if (oldBounds.size.height > 0)
  {
    auto yScale = newFrame.size.height / oldBounds.size.height;
    boundsJson[K_Y] = oldBounds.origin.y * yScale;
    matrix.ty *= yScale;
  }
  boundsJson[K_WIDTH] = newFrame.size.width;
  boundsJson[K_HEIGHT] = newFrame.size.height;
  DEBUG(
    "LayoutNode::updateModel: bounds, -> %s, %s",
    objectJson[K_BOUNDS].dump().c_str(),
    boundsJson.dump().c_str());
  viewModel->replaceAt(path / K_BOUNDS, boundsJson);

  auto matrixJson = objectJson[K_MATRIX];
  matrixJson[4] = matrix.tx;
  matrixJson[5] = matrix.ty;
  DEBUG(
    "LayoutNode::updateModel: matrix, -> %s, %s",
    objectJson[K_MATRIX].dump().c_str(),
    matrixJson.dump().c_str());
  viewModel->replaceAt(path / K_MATRIX, matrixJson);

  // translate if needed
  auto originAfterScale = modelOrigin();
  if (originAfterScale != newFrame.origin)
  {
    matrix.tx += newFrame.origin.x - originAfterScale.x;
    matrix.ty += newFrame.origin.y - originAfterScale.y;
    matrixJson[4] = matrix.tx;
    matrixJson[5] = matrix.ty;
    DEBUG(
      "LayoutNode::updateModel: matrix, -> %s, %s",
      objectJson[K_MATRIX].dump().c_str(),
      matrixJson.dump().c_str());
    viewModel->replaceAt(path / K_MATRIX, matrixJson);
  }
}

Layout::Point LayoutNode::origin() const
{
  return modelOrigin().makeFromModelPoint();
}

Layout::Point LayoutNode::modelOrigin() const
{
  auto viewModel = m_viewModel.lock();
  if (!viewModel)
  {
    return {};
  }

  auto bounds = modelBounds();
  auto matrix = modelMatrix();
  auto [x, y] = bounds.origin;
  auto [a, b, c, d, tx, ty] = matrix;
  auto newX = a * x + c * y + tx;
  auto newY = b * x + d * y + ty;

  Layout::Point relativePosition{ newX, newY };
  if (auto parent = m_parent.lock())
  {
    const auto& parentOrigin = parent->modelBounds().origin;
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

  auto parent = m_parent.lock();
  while (parent && parent != ancestorNode)
  {
    auto origin = parent->origin();
    x += origin.x;
    y += origin.y;
    parent = parent->m_parent.lock();
  }

  return { x, y };
}

void LayoutNode::scaleTo(const Layout::Size& newSize, bool updateRule, bool preservingOrigin)
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
  autoLayout()->setNeedsLayout();
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

  if (auto json = model(); Layout::isContourPathNode(*json))
  {
    return resizeContour(oldContainerSize, newContainerSize, parentOrigin);
  }

  auto oldFrame = frame();
  if (isResizingAroundCenter())
  {
    Layout::Point center{ oldFrame.width() / 2, oldFrame.height() / 2 };
    center = center.makeModelPoint().makeTransform(modelMatrix()).makeFromModelPoint();
    oldFrame.setCenter(center);
  }
  auto [x, w] = resizeH(oldContainerSize, newContainerSize, oldFrame, parentOrigin);
  auto [y, h] = resizeV(oldContainerSize, newContainerSize, oldFrame, parentOrigin);
  Layout::Point newOrigin{ x, y };
  Layout::Rect  newFrame{ newOrigin, { w, h } };
  if (isResizingAroundCenter())
  {
    newOrigin = (newOrigin - newFrame.center())
                  .makeModelPoint()
                  .makeTransform(modelMatrixWithoutTranslate())
                  .makeFromModelPoint() +
                newFrame.center();
    newFrame.origin = newOrigin;
  }

  if (!dry)
  {
    setFrame(newFrame, false, true);
  }

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

  const auto rightMargin = oldContainerSize.width - oldFrame.right();

  switch (horizontalResizing())
  {
    case EResizing::FIX_START_FIX_END:
    {
      x = oldFrame.left();
      w = newContainerSize.width - rightMargin - x;
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
      w = oldFrame.width() * (newContainerSize.width - x) /
          (oldContainerSize.width - oldFrame.left());
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
      w = oldFrame.width() * (newContainerSize.width - rightMargin) /
          (oldContainerSize.width - rightMargin);
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

  const auto bottomMargin = oldContainerSize.height - oldFrame.bottom();

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
      h = oldFrame.height() * (newContainerSize.height - y) /
          (oldContainerSize.height - oldFrame.top());
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
      h = oldFrame.height() * (newContainerSize.height - bottomMargin) /
          (oldContainerSize.height - bottomMargin);
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

  return { y, h };
}

LayoutNode::EResizing LayoutNode::horizontalResizing() const
{
  if (isVectorNetworkDescendant())
  {
    return EResizing::SCALE;
  }

  return getValue(K_HORIZONTAL_CONSTRAINT, EResizing::FIX_START_FIX_SIZE);
}

LayoutNode::EResizing LayoutNode::verticalResizing() const
{
  if (isVectorNetworkDescendant())
  {
    return EResizing::SCALE;
  }

  return getValue(K_VERTICAL_CONSTRAINT, EResizing::FIX_START_FIX_SIZE);
}

LayoutNode::EAdjustContentOnResize LayoutNode::adjustContentOnResize() const
{
  return getValue(K_RESIZES_CONTENT, EAdjustContentOnResize::ENABLED);
}

template<typename T>
T LayoutNode::getValue(const char* key, T v) const
{
  if (auto json = model())
  {
    return json->value(key, v);
  }

  return v;
}

const nlohmann::json* LayoutNode::model() const
{
  auto viewModel = m_viewModel.lock();
  if (!viewModel)
  {
    return nullptr;
  }

  nlohmann::json::json_pointer path{ m_path };
  const auto&                  objectJson = viewModel->content()[path];

  return &objectJson;
}

bool LayoutNode::shouldSkip()
{
  // parent adjust content is skip group or boolean group && (group || (boolean group))
  if (auto parent = m_parent.lock();
      parent &&
      parent->adjustContentOnResize() == EAdjustContentOnResize::SKIP_GROUP_OR_BOOLEAN_GROUP)
  {
    if (isVectorNetwork())
    {
      return false;
    }

    auto json = model();
    if (Layout::isGroupNode(*json))
    {
      return true;
    }

    return isBooleanGroup();
  }

  return false;
}

bool LayoutNode::isBooleanGroup()
{
  auto json = model();
  return Layout::isPathNode(*json) && m_children.size() > 1;
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

  if (auto json = model(); json && isBooleanGroup())
  {
    auto& subShapes = (*json)[K_SHAPE][K_SUBSHAPES];

    newGroupFrame = childTransformedFrames[0];
    for (std::size_t i = 1; i < m_children.size(); i++)
    {
      auto& subShape = subShapes[i];
      auto  op = subShape.value(K_BOOLEANOPERATION, EBooleanOperation::NONE);
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
  if (auto parent = m_parent.lock())
  {
    const auto& parentOrigin = parent->modelBounds().origin;
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

  const auto parentNode = m_parent.lock();
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

Layout::Rect LayoutNode::resizeContour(
  const Layout::Size&  oldContainerSize,
  const Layout::Size&  newContainerSize,
  const Layout::Point* parentOrigin)
{
  DEBUG("resizeContour: begin, name = %s, id = %s", name().c_str(), id().c_str());

  const auto json = model();
  ASSERT(json);

  const auto& subshapes = (*json)[K_SHAPE][K_SUBSHAPES];
  ASSERT(subshapes.size() == 1);

  const auto& subshape = subshapes[0];
  const auto& subGeometry = subshape[K_SUBGEOMETRY];
  ASSERT(subGeometry[K_CLASS] == K_CONTOUR);

  // get model point
  const auto& points = subGeometry[K_POINTS];
  ASSERT(points.size() > 0);
  const bool isClosed = subGeometry[K_CLOSED];

  // const auto                       oldSize = modelBounds().size;
  std::vector<Layout::BezierPoint> oldModelPoints;
  for (std::size_t j = 0; j < points.size(); ++j)
  {
    auto point = points[j].get<Layout::BezierPoint>();
    oldModelPoints.push_back(point);

    // auto oldFlipYModelPoint = point.point.makeFromModelPoint();
    // DEBUG(
    //   "resizeContour: old flip y model point %d: %f, %f",
    //   j,
    //   oldFlipYModelPoint.x,
    //   oldFlipYModelPoint.y);
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

void LayoutNode::scaleContour(const Layout::Rect& oldFrame, const Layout::Rect& newFrame)
{
  DEBUG("LayoutNode::scaleContour, id=%s", id().c_str());

  const auto oldSize = oldFrame.size;
  const auto newSize = newFrame.size;

  const auto xScaleFactor = doubleNearlyZero(oldSize.width) ? 0 : newSize.width / oldSize.width;
  const auto yScaleFactor = doubleNearlyZero(oldSize.height) ? 0 : newSize.height / oldSize.height;

  const auto nodeJson = model();
  ASSERT(nodeJson);

  // ./shape/subshapes/XXX/subGeometry/points/XXX
  auto& subshapes = (*nodeJson)[K_SHAPE][K_SUBSHAPES];
  for (std::size_t i = 0; i < subshapes.size(); ++i)
  {
    auto& subshape = subshapes[i];
    auto& subGeometry = subshape[K_SUBGEOMETRY];
    if (subGeometry[K_CLASS] != K_CONTOUR)
    {
      continue;
    }

    std::vector<Layout::BezierPoint> newModelPoints;
    auto&                            points = subGeometry[K_POINTS];
    for (std::size_t j = 0; j < points.size(); ++j)
    {
      auto point = points[j].get<Layout::BezierPoint>();
      point.scale(xScaleFactor, yScaleFactor);

      newModelPoints.push_back(point);
    }
    updateContourNodeModelPoints(i, newModelPoints);
  }
}

void LayoutNode::updatePathNodeModel(
  const Layout::Rect&                     newFrame,
  const Layout::Matrix&                   matrix,
  const std::vector<Layout::BezierPoint>& newPoints)
{
  auto viewModel = m_viewModel.lock();
  if (!viewModel)
  {
    return;
  }

  nlohmann::json::json_pointer path{ m_path };
  const auto&                  objectJson = viewModel->content()[path];

  if (objectJson[K_CLASS] != K_PATH)
  {
    return;
  }
  // do not update useless frame

  // bounds
  auto boundsJson = objectJson[K_BOUNDS];
  boundsJson[K_WIDTH] = newFrame.width();
  boundsJson[K_HEIGHT] = newFrame.height();
  DEBUG(
    "LayoutNode::updatePathNodeModel: bounds, -> %s, %s",
    objectJson[K_BOUNDS].dump().c_str(),
    boundsJson.dump().c_str());
  viewModel->replaceAt(path / K_BOUNDS, boundsJson);

  // matrix
  auto matrixJson = objectJson[K_MATRIX];
  matrixJson[0] = matrix.a;
  matrixJson[1] = matrix.b;
  matrixJson[2] = matrix.c;
  matrixJson[3] = matrix.d;
  matrixJson[4] = matrix.tx;
  matrixJson[5] = matrix.ty;
  DEBUG(
    "LayoutNode::updatePathNodeModel: matrix, -> %s, %s",
    objectJson[K_MATRIX].dump().c_str(),
    matrixJson.dump().c_str());
  viewModel->replaceAt(path / K_MATRIX, matrixJson);

  // points
  // ./shape/subshapes/XXX/subGeometry/points/XXX
  auto& subshapes = objectJson[K_SHAPE][K_SUBSHAPES];
  path /= K_SHAPE;
  path /= K_SUBSHAPES;
  for (std::size_t i = 0; i < subshapes.size(); ++i)
  {
    auto& subshape = subshapes[i];
    auto& subGeometry = subshape[K_SUBGEOMETRY];
    if (subGeometry[K_CLASS] != K_CONTOUR)
    {
      continue;
    }

    auto& points = subGeometry[K_POINTS];
    auto  pointsPath = path / i / K_SUBGEOMETRY / K_POINTS;
    for (std::size_t j = 0; j < points.size(); ++j)
    {
      auto point = points[j];
      to_json(point, newPoints[j]);

      auto pointPath = pointsPath / j;
      DEBUG(
        "LayoutNode::updatePathNodeModel: %s, -> %s, %s",
        pointPath.to_string().c_str(),
        points[j].dump().c_str(),
        point.dump().c_str());

      viewModel->replaceAt(pointPath, point);
    }
  }
}

void LayoutNode::updateContourNodeModelPoints(
  const std::size_t                       subshapeIndex,
  const std::vector<Layout::BezierPoint>& newPoints)
{
  auto viewModel = m_viewModel.lock();
  if (!viewModel)
  {
    return;
  }

  auto json = model();
  if (!json)
  {
    return;
  }

  // points
  // ./shape/subshapes/XXX/subGeometry/points/XXX
  nlohmann::json::json_pointer path{ m_path };
  auto&                        subshapes = (*json)[K_SHAPE][K_SUBSHAPES];
  path /= K_SHAPE;
  path /= K_SUBSHAPES;

  auto& subshape = subshapes[subshapeIndex];
  auto& subGeometry = subshape[K_SUBGEOMETRY];

  auto& points = subGeometry[K_POINTS];
  auto  pointsPath = path / subshapeIndex / K_SUBGEOMETRY / K_POINTS;
  for (std::size_t j = 0; j < points.size(); ++j)
  {
    auto point = points[j];
    to_json(point, newPoints[j]);

    auto pointPath = pointsPath / j;
    DEBUG(
      "LayoutNode::updateContourNodeModelPoints: %s, -> %s, %s",
      pointPath.to_string().c_str(),
      points[j].dump().c_str(),
      point.dump().c_str());

    viewModel->replaceAt(pointPath, point);
  }
}

void LayoutNode::removeAllChildren()
{
  autoLayout()->removeSubtree();
  m_children.clear();
}

} // namespace VGG
