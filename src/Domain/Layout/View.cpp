#include "View.hpp"

#include "flexbox_node.h"
#include "grid_layout.h"

namespace VGG
{

void LayoutView::configureFlexLayout(const Layout::Internal::ConfigureFlexLayoutFn& configureFn)
{
  if (configureFn)
  {
    m_grid_node.reset();
    m_flex_node.reset(new flexbox_node);

    configureFn(m_flex_node);
  }
}

void LayoutView::configureGridLayout(const Layout::Internal::ConfigureGridLayoutFn& configureFn)
{
  if (configureFn)
  {
    m_flex_node.reset();

    auto column_count = 0; // todo
    m_grid_node.reset(new grid_layout(column_count));

    configureFn(m_grid_node);
  }
}

void LayoutView::applyLayout()
{
}

} // namespace VGG