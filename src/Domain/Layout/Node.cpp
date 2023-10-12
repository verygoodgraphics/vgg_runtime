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

const Layout::Rect& LayoutNode::frame() const
{
  return m_frame;
}

void LayoutNode::setFrame(const Layout::Rect& frame, bool updateRule)
{
  if (m_frame != frame)
  {
    DEBUG("LayoutNode::setFrame: %s, %s, %s, %4d, %4d, %4d, %4d -> %4d, %4d, %4d, %4d",
          id().c_str(),
          path().c_str(),
          m_autoLayout->isEnabled() ? "layout" : "scale",
          static_cast<int>(m_frame.origin.x),
          static_cast<int>(m_frame.origin.y),
          static_cast<int>(m_frame.size.width),
          static_cast<int>(m_frame.size.height),
          static_cast<int>(frame.origin.x),
          static_cast<int>(frame.origin.y),
          static_cast<int>(frame.size.width),
          static_cast<int>(frame.size.height));

    auto oldSize = m_frame.size;
    auto newSize = frame.size;

    m_frame = frame;

    // updateAt
    if (auto viewModel = m_viewModel.lock())
    {
      nlohmann::json::json_pointer path{ m_path };
      const auto& objectJson = viewModel->content()[path];
      auto frameJson = objectJson[K_FRAME];
      auto boundsJson = objectJson[K_BOUNDS];
      auto matrixJson = objectJson[K_MATRIX];

      const Layout::Rect vggFrame{ { frame.origin.x, frame.origin.y * VGG::Layout::FLIP_Y_FACTOR },
                                   frame.size };

      to_json(frameJson, vggFrame);

      boundsJson[K_WIDTH] = vggFrame.size.width;
      boundsJson[K_HEIGHT] = vggFrame.size.height;

      matrixJson[4] = vggFrame.origin.x;
      matrixJson[5] = vggFrame.origin.y;

      viewModel->replaceAt(path / K_FRAME, frameJson);
      viewModel->replaceAt(path / K_BOUNDS, boundsJson);
      viewModel->replaceAt(path / K_MATRIX, matrixJson);
    }

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

      if (!m_autoLayout->isContainer())
      {
        scaleChildNodes(oldSize, newSize);
      }
    }
    else
    {
      scaleChildNodes(oldSize, newSize);
    }
  }
}

const Layout::Rect& LayoutNode::bounds() const
{
  return m_bounds;
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

void LayoutNode::scaleChildNodes(const Layout::Size& oldSize, const Layout::Size& newSize)
{
  if (oldSize == Layout::Size{ 0, 0 })
  {
    return;
  }

  auto xScaleFactor = newSize.width / oldSize.width;
  auto yScaleFactor = newSize.height / oldSize.height;
  for (auto& child : m_children)
  {
    auto frame = child->frame();

    // todo, support more constraints
    frame.origin.x *= xScaleFactor;
    frame.origin.y *= yScaleFactor;
    frame.size.width *= xScaleFactor;
    frame.size.height *= yScaleFactor;

    child->setFrame(frame);
  }

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

} // namespace VGG
