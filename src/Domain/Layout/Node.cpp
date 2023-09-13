#include "Node.hpp"

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Rect.hpp"

#include <Log.h>

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

void LayoutNode::applyLayout()
{
  for (auto subview : m_children)
  {
    subview->applyLayout();
  }

  if (m_autoLayout)
  {
    m_autoLayout->applyLayout(false);
  }
}

void LayoutNode::setNeedLayout()
{
  m_needsLayout = true;
}

void LayoutNode::layoutIfNeeded()
{
  for (auto subview : m_children)
  {
    subview->layoutIfNeeded();
  }

  if (m_needsLayout)
  {
    m_needsLayout = false;

    configureAutoLayout();
    applyLayout();
  }
}

const Layout::Rect& LayoutNode::frame() const
{
  return m_frame;
}
void LayoutNode::setFrame(const Layout::Rect& frame)
{
  if (m_frame != frame)
  {
    DEBUG("LayoutNode::setFrame: %s, %4d, %4d, %4d, %4d -> %4d, %4d, %4d, %4d, %s",
          m_autoLayout->isEnabled() ? "layout" : "scale",
          static_cast<int>(m_frame.origin.x),
          static_cast<int>(m_frame.origin.y),
          static_cast<int>(m_frame.size.width),
          static_cast<int>(m_frame.size.height),
          static_cast<int>(frame.origin.x),
          static_cast<int>(frame.origin.y),
          static_cast<int>(frame.size.width),
          static_cast<int>(frame.size.height),
          path().c_str());

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

      to_json(frameJson, frame);

      boundsJson[K_WIDTH] = frame.size.width;
      boundsJson[K_HEIGHT] = frame.size.height;

      matrixJson[4] = frame.origin.x;
      matrixJson[5] = frame.origin.y * VGG::Layout::FLIP_Y_FACTOR;

      viewModel->replaceAt(path / K_FRAME, frameJson);
      viewModel->replaceAt(path / K_BOUNDS, boundsJson);
      viewModel->replaceAt(path / K_MATRIX, matrixJson);
    }

    if (newSize == oldSize)
    {
      return;
    }

    if (m_autoLayout->isEnabled() && m_autoLayout->isContainer())
    {
      m_autoLayout->frameChanged();
    }
    else
    {
      scaleChildNodes(oldSize, newSize);
    }
  }
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

} // namespace VGG