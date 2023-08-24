#include "View.hpp"

#include "Bridge.hpp"

namespace VGG
{

void LayoutView::configureLayout(const Layout::Internal::ConfigureLayoutFn& configureFn)
{
  if (configureFn)
  {
    m_bridge.reset(new Layout::Internal::Bridge);
    m_bridge->view = weak_from_this();

    configureFn(m_bridge);
  }
}

void LayoutView::applyLayout()
{
  if (m_bridge)
  {
    m_bridge->applyLayout(false);
  }
}

} // namespace VGG