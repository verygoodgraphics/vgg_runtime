#include "Bridge.hpp"

#include "Log.h"
#include "View.hpp"

#include <memory>
#include <vector>

namespace VGG
{
class LayoutView;

namespace Layout
{

namespace Internal
{

namespace
{
using Views = std::vector<std::shared_ptr<LayoutView>>;

void removeAllChildren(flexbox_node* node)
{
  while (node->child_count() > 0)
  {
    node->remove_child(node->child_count() - 1);
  }
}

bool layoutNodeHasExactSameChildren(flexbox_node* node, Views subviews)
{
  if (node->child_count() != subviews.size())
  {
    return false;
  }

  for (auto i = 0; i < subviews.size(); ++i)
  {
    if (node->get_child(i) != subviews[i]->layoutBridge()->flexNode.get())
    {
      return false;
    }
  }

  return true;
}

void attachNodesFromViewHierachy(std::shared_ptr<LayoutView> view)
{
  // todo
  const auto bridge = view->layoutBridge();
  if (bridge->flexNode)
  {
    const auto node = bridge->flexNode;
    if (bridge->isLeaf())
    {
      removeAllChildren(node.get());
    }
    else
    {
      std::vector<std::shared_ptr<LayoutView>> subviewsToInclude;
      for (auto subview : view->children())
      {
        if (subview->layoutBridge()->isEnabled && subview->layoutBridge()->isIncludedInLayout)
        {
          subviewsToInclude.push_back(subview);
        }
      }

      if (!layoutNodeHasExactSameChildren(node.get(), subviewsToInclude))
      {
        removeAllChildren(node.get());
        for (auto i = 0; i < subviewsToInclude.size(); ++i)
        {
          // todo
          // subviewsToInclude[i]->layoutBridge()->flexNode,
          node->add_child(i);
        }
      }

      for (auto subview : subviewsToInclude)
      {
        attachNodesFromViewHierachy(subview);
      }
    }
  }
}

void applyLayoutToViewHierarchy(std::shared_ptr<LayoutView> view, bool preserveOrigin)
{
  auto layoutBridge = view->layoutBridge();
  if (!layoutBridge->isIncludedInLayout)
  {
    return;
  }

  auto node = layoutBridge->flexNode;
  Point origin;
  if (preserveOrigin)
  {
    origin = view->frame().origin;
  }
  view->setFrame(
    { { .x = node->get_layout_left() + origin.x, .y = node->get_layout_top() + origin.y },
      { .width = node->get_layout_width(), .height = node->get_layout_height() } });

  if (!layoutBridge->isLeaf())
  {
    for (auto subview : view->children())
    {
      applyLayoutToViewHierarchy(subview, false);
    }
  }

  // todo, gridNode
}

} // namespace

void Bridge::applyLayout(bool preservingOrigin)
{
  if (auto sharedView = view.lock())
  {
    calculateLayout(sharedView->frame().size);
    applyLayoutToViewHierarchy(sharedView, preservingOrigin);
  }
}

Size Bridge::calculateLayout(Size size)
{
  if (auto sharedView = view.lock())
  {
    attachNodesFromViewHierachy(sharedView);
  }

  if (flexNode)
  {
    flexNode->calc_layout();
    return { flexNode->get_layout_width(), flexNode->get_layout_height() };
  }
  else if (gridNode)
  {
    gridNode->calc_layout();
    return { TO_VGG_LAYOUT_SCALAR(gridNode->get_layout_width()),
             TO_VGG_LAYOUT_SCALAR(gridNode->get_layout_height()) };
  }
  else
  {
    WARN("Bridge::calculateLayout, no layout node, return parameter size");
    return size;
  }
}

bool Bridge::isLeaf()
{
  if (isEnabled)
  {
    if (auto sharedView = view.lock())
    {
      for (auto& child : sharedView->children())
      {
        auto bridge = child->layoutBridge();
        if (bridge->isEnabled && bridge->isIncludedInLayout)
        {
          return false;
        }
      }
    }
  }

  return true;
}

} // namespace Internal
} // namespace Layout
} // namespace VGG