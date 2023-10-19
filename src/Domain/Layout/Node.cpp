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
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Rect.hpp"

#include "Utility/Log.hpp"

#undef DEBUG
#define DEBUG(msg, ...)

namespace VGG
{

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
  auto oldFrame = useOldFrame ? m_oldFrame : frame();
  if (oldFrame == newFrame)
  {
    return;
  }

  DEBUG("LayoutNode::setFrame: %s, %s, %s, %4d, %4d, %4d, %4d -> %4d, %4d, %4d, %4d",
        id().c_str(),
        path().c_str(),
        m_autoLayout->isEnabled() ? "layout" : "scale",
        static_cast<int>(oldFrame.origin.x),
        static_cast<int>(oldFrame.origin.y),
        static_cast<int>(oldFrame.size.width),
        static_cast<int>(oldFrame.size.height),
        static_cast<int>(newFrame.origin.x),
        static_cast<int>(newFrame.origin.y),
        static_cast<int>(newFrame.size.width),
        static_cast<int>(newFrame.size.height));

  saveChildrendOldFrame();
  updateModel(newFrame);

  auto oldSize = oldFrame.size;
  auto newSize = newFrame.size;
  if (newSize == oldSize)
  {
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
      return;
    }
  }

  scaleChildNodes(oldSize, newSize, true);
}

Layout::Rect LayoutNode::bounds() const
{
  return modelBounds().fromModelRect();
}

Layout::Rect LayoutNode::modelBounds() const
{
  Layout::Rect bounds;

  auto viewModel = m_viewModel.lock();
  if (viewModel)
  {
    nlohmann::json::json_pointer path{ m_path };
    const auto& objectJson = viewModel->content()[path];
    bounds = objectJson[K_BOUNDS].get<Layout::Rect>();
  }

  return bounds;
}

Layout::Size LayoutNode::size() const
{
  return modelBounds().size;
}

void LayoutNode::setViewModel(JsonDocumentPtr viewModel)
{
  m_viewModel = viewModel;
}

void LayoutNode::configureAutoLayout()
{
  if (m_autoLayout)
  {
    m_autoLayout->configure();
  }
}

void LayoutNode::scaleChildNodes(const Layout::Size& oldContainerSize,
                                 const Layout::Size& newContainerSize,
                                 bool useOldFrame)
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
    child->resize(oldContainerSize, newContainerSize, useOldFrame);
  }

  auto xScaleFactor = newContainerSize.width / oldContainerSize.width;
  auto yScaleFactor = newContainerSize.height / oldContainerSize.height;
  scaleContour(xScaleFactor, yScaleFactor);
}

void LayoutNode::scaleContour(float xScaleFactor, float yScaleFactor)
{
  // updateAt
  auto viewModel = m_viewModel.lock();
  if (!viewModel)
  {
    return;
  }

  nlohmann::json::json_pointer path{ m_path };
  const auto& objectJson = viewModel->content()[path];
  if (objectJson[K_CLASS] != K_PATH)
  {
    return;
  }

  // ./shape/subshapes/XXX/subGeometry/points/XXX
  auto& subshapes = objectJson[K_SHAPE][K_SUBSHAPES];
  path /= K_SHAPE;
  path /= K_SUBSHAPES;
  for (auto i = 0; i < subshapes.size(); ++i)
  {
    auto& subshape = subshapes[i];
    auto& subGeometry = subshape[K_SUBGEOMETRY];
    if (subGeometry[K_CLASS] != K_CONTOUR)
    {
      continue;
    }

    auto& points = subGeometry[K_POINTS];
    auto pointsPath = path / i / K_SUBGEOMETRY / K_POINTS;
    for (auto j = 0; j < points.size(); ++j)
    {
      auto point = points[j];
      scalePoint(point, K_POINT, xScaleFactor, yScaleFactor);
      scalePoint(point, K_CURVE_FROM, xScaleFactor, yScaleFactor);
      scalePoint(point, K_CURVE_TO, xScaleFactor, yScaleFactor);

      auto pointPath = pointsPath / j;
      DEBUG("LayoutNode::scaleContour: %s, -> %s, %s",
            pointPath.to_string().c_str(),
            points[j].dump().c_str(),
            point.dump().c_str());

      viewModel->replaceAt(pointPath, point);
    }
  }
}

void LayoutNode::scalePoint(nlohmann::json& json,
                            const char* key,
                            float xScaleFactor,
                            float yScaleFactor)
{
  if (json.contains(key))
  {
    auto point = json[key].get<Layout::Point>();

    point.x *= xScaleFactor;
    point.y *= yScaleFactor;

    json[key] = point;
  }
}

std::string LayoutNode::id()
{
  if (auto docJson = m_viewModel.lock())
  {
    nlohmann::json::json_pointer path{ m_path };
    return docJson->content()[path][K_ID];
  }

  return {};
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

  const auto newFrame = toFrame.toModelRect();

  nlohmann::json::json_pointer path{ m_path };
  const auto& objectJson = viewModel->content()[path];

  // do not update useless frame

  // scale, bounds & matrix
  auto boundsJson = objectJson[K_BOUNDS];
  auto oldBounds = modelBounds();
  auto matrix = objectJson[K_MATRIX].get<Layout::Matrix>();
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
  viewModel->replaceAt(path / K_BOUNDS, boundsJson);

  auto matrixJson = objectJson[K_MATRIX];
  matrixJson[4] = matrix.tx;
  matrixJson[5] = matrix.ty;
  viewModel->replaceAt(path / K_MATRIX, matrixJson);

  // translate if needed
  auto originAfterScale = modelOrigin();
  if (originAfterScale != newFrame.origin)
  {
    matrix.tx += newFrame.origin.x - originAfterScale.x;
    matrix.ty += newFrame.origin.y - originAfterScale.y;
    matrixJson[4] = matrix.tx;
    matrixJson[5] = matrix.ty;
    viewModel->replaceAt(path / K_MATRIX, matrixJson);
  }
}

Layout::Point LayoutNode::origin() const
{
  return modelOrigin().fromModelPoint();
}

Layout::Point LayoutNode::modelOrigin() const
{
  auto viewModel = m_viewModel.lock();
  if (!viewModel)
  {
    return {};
  }

  nlohmann::json::json_pointer path{ m_path };
  const auto& j = viewModel->content()[path];

  auto bounds = j[K_BOUNDS].get<Layout::Rect>();
  auto matrix = j[K_MATRIX].get<Layout::Matrix>();
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

Layout::Point LayoutNode::converPointToAncestor(Layout::Point point,
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

void LayoutNode::scaleTo(const Layout::Size& newSize, bool updateRule)
{
  auto oldFrame = frame();
  auto oldSize = oldFrame.size;
  auto newFrame = oldFrame;
  if (oldSize.width > 0)
  {
    auto xScale = newSize.width / oldSize.width;
    newFrame.origin.x *= xScale;
  }
  if (oldSize.height > 0)
  {
    auto yScale = newSize.height / oldSize.height;
    newFrame.origin.y *= yScale;
  }
  newFrame.size = newSize;

  saveOldFrame();
  setFrame(newFrame, updateRule, true);
  autoLayout()->setNeedsLayout();
}

void LayoutNode::saveOldFrame()
{
  m_oldFrame = frame();
}

void LayoutNode::saveChildrendOldFrame()
{
  for (auto& child : m_children)
  {
    child->saveOldFrame();
  }
}

void LayoutNode::resize(const Layout::Size& oldContainerSize,
                        const Layout::Size& newContainerSize,
                        bool useOldFrame)
{
  auto [x, w] = resizeH(oldContainerSize, newContainerSize, useOldFrame);
  auto [y, h] = resizeV(oldContainerSize, newContainerSize, useOldFrame);
  Layout::Rect newFrame{ { x, y }, { w, h } };

  setFrame(newFrame, false, useOldFrame);
}

std::pair<Layout::Scalar, Layout::Scalar> LayoutNode::resizeH(const Layout::Size& oldContainerSize,
                                                              const Layout::Size& newContainerSize,
                                                              bool useOldFrame) const
{
  Layout::Scalar x{ 0 }, w{ 0 };

  const auto oldFrame = useOldFrame ? m_oldFrame : frame();
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

std::pair<Layout::Scalar, Layout::Scalar> LayoutNode::resizeV(const Layout::Size& oldContainerSize,
                                                              const Layout::Size& newContainerSize,
                                                              bool useOldFrame) const
{
  Layout::Scalar y{ 0 }, h{ 0 };

  const auto oldFrame = useOldFrame ? m_oldFrame : frame();
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
  // todo, check ancestor's resizing content flag
  return getValue(K_HORIZONTAL_CONSTRAINT, EResizing::FIX_START_FIX_SIZE);
}

LayoutNode::EResizing LayoutNode::verticalResizing() const
{
  // todo, check ancestor's resizing content flag
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
  const auto& objectJson = viewModel->content()[path];

  return &objectJson;
}

} // namespace VGG
