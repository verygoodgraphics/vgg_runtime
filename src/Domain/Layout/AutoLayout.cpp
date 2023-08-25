#include "AutoLayout.hpp"

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
    if (node->get_child(i) != subviews[i]->autoLayout()->getFlexNode())
    {
      return false;
    }
  }

  return true;
}

void attachNodesFromViewHierachy(std::shared_ptr<LayoutView> view)
{
  // todo
  const auto autoLayout = view->autoLayout();
  if (const auto node = autoLayout->getFlexNode())
  {
    if (autoLayout->isLeaf())
    {
      removeAllChildren(node);
    }
    else
    {
      std::vector<std::shared_ptr<LayoutView>> subviewsToInclude;
      for (auto subview : view->children())
      {
        if (subview->autoLayout()->isEnabled() && subview->autoLayout()->isIncludedInLayout)
        {
          subviewsToInclude.push_back(subview);
        }
      }

      if (!layoutNodeHasExactSameChildren(node, subviewsToInclude))
      {
        removeAllChildren(node);
        for (auto i = 0; i < subviewsToInclude.size(); ++i)
        {
          DEBUG("attachNodesFromViewHierachy, flex node add child");
          node->add_child(subviewsToInclude[i]->autoLayout()->takeFlexNode(), i);
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
  auto autoLayout = view->autoLayout();
  if (!autoLayout->isIncludedInLayout)
  {
    return;
  }

  if (auto node = autoLayout->getFlexNode())
  {
    Point origin;
    if (preserveOrigin)
    {
      origin = view->frame().origin;
    }
    Layout::Rect frame = {
      { .x = node->get_layout_left() + origin.x, .y = node->get_layout_top() + origin.y },
      { .width = node->get_layout_width(), .height = node->get_layout_height() }
    };

    DEBUG("applyLayoutToViewHierarchy, view[%p, %s], x=%d, y=%d, width=%d, height=%d",
          view.get(),
          view->path().c_str(),
          static_cast<int>(frame.origin.x),
          static_cast<int>(frame.origin.y),
          static_cast<int>(frame.size.width),
          static_cast<int>(frame.size.height));

    autoLayout->frame = frame;
    view->setFrame(frame);
  }

  if (!autoLayout->isLeaf())
  {
    for (auto subview : view->children())
    {
      applyLayoutToViewHierarchy(subview, false);
    }
  }

  // todo, gridNode
}

} // namespace

void AutoLayout::applyLayout(bool preservingOrigin)
{
  if (isLeaf())
  {
    return;
  }

  if (auto sharedView = view.lock())
  {
    calculateLayout(sharedView->frame().size);
    applyLayoutToViewHierarchy(sharedView, preservingOrigin);
  }
}

Size AutoLayout::calculateLayout(Size size)
{
  if (auto sharedView = view.lock())
  {
    DEBUG("AutoLayout::calculateLayout, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());
    attachNodesFromViewHierachy(sharedView);
  }

  if (auto flexNode = getFlexNode())
  {
    flexNode->calc_layout();
    return { flexNode->get_layout_width(), flexNode->get_layout_height() };
  }
  else if (auto gridNode = getGridNode())
  {
    DEBUG("AutoLayout::calculateLayout, grid node calculate");
    gridNode->calc_layout();
    return { TO_VGG_LAYOUT_SCALAR(gridNode->get_layout_width()),
             TO_VGG_LAYOUT_SCALAR(gridNode->get_layout_height()) };
  }
  else
  {
    WARN("AutoLayout::calculateLayout, no layout node, return parameter size");
    return size;
  }
}

bool AutoLayout::isLeaf()
{
  if (isEnabled())
  {
    if (auto sharedView = view.lock())
    {
      for (auto& child : sharedView->children())
      {
        auto autoLayout = child->autoLayout();
        if (autoLayout->isEnabled() && autoLayout->isIncludedInLayout)
        {
          return false;
        }
      }
    }
  }

  return true;
}

void AutoLayout::frameChanged()
{
  auto sharedView = view.lock();
  auto sharedRule = rule.lock();
  if (isEnabled() && sharedView && sharedRule)
  {
    auto newSize = sharedView->frame().size;
    if (newSize != frame.size)
    {
      // todo, old type is percent
      sharedRule->width.value.types = Rule::Length::Types::px;
      sharedRule->width.value.value = newSize.width;

      sharedRule->height.value.types = Rule::Length::Types::px;
      sharedRule->height.value.value = newSize.height;

      if (sharedRule->isFlexConatiner() || sharedRule->isFlexConatiner())
      {
        sharedView->setNeedLayout();
      }
      else if (sharedRule->isFlexItem() || sharedRule->isGridItem())
      {
        if (auto container = sharedView->autoLayoutContainer())
        {
          container->setNeedLayout();
        }

        // todo, scale descendant node
      }
    }
  }
}

std::unique_ptr<flexbox_node>& AutoLayout::takeFlexNode()
{
  return m_flexNode;
}

std::unique_ptr<grid_layout>& AutoLayout::takeGridNode()
{
  return m_gridNode;
}

flexbox_node* AutoLayout::createFlexNode()
{
  m_flexNodePtr = new flexbox_node;
  m_flexNode.reset(m_flexNodePtr);
  return m_flexNode.get();
}

grid_layout* AutoLayout::createGridNode()
{
  // todo
  return nullptr;
}

flexbox_node* AutoLayout::getFlexNode()
{
  return m_flexNodePtr;
}

grid_layout* AutoLayout::getGridNode()
{
  return m_gridNodePtr;
}

} // namespace Internal
} // namespace Layout
} // namespace VGG