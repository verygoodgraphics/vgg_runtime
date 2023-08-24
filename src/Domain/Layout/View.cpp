#include "View.hpp"

#include "Bridge.hpp"

namespace VGG
{

std::shared_ptr<Layout::Internal::Bridge> LayoutView::configureLayout()
{
  m_bridge.reset(new Layout::Internal::Bridge);
  m_bridge->view = weak_from_this();

  return m_bridge;
}

std::shared_ptr<flexbox_node> LayoutView::createFlexboxNode()
{
  flexbox_node::p_node node{ new flexbox_node };
  m_bridge->flexNode = node;

  return m_bridge->flexNode;
}

std::shared_ptr<grid_layout> LayoutView::createGridNode()
{
  // todo
  return m_bridge->gridNode;
}

void LayoutView::applyLayout()
{
  for (auto subview : m_children)
  {
    subview->applyLayout();
  }

  if (m_bridge)
  {
    m_bridge->applyLayout(false);
  }
}

} // namespace VGG