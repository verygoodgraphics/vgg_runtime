#include "Node.hpp"

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"

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
    auto oldSize = m_frame.size;
    auto newSize = frame.size;

    m_frame = frame;

    // updateAt
    if (auto viewModel = m_viewModel.lock())
    {
      nlohmann::json::json_pointer path{ m_path };
      nlohmann::json objectJson{ viewModel->content()[path] };
      auto& frameJson = objectJson[K_FRAME];
      auto& boundsJson = objectJson[K_BOUNDS];
      auto& matrixJson = objectJson[K_MATRIX];

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

    if (m_autoLayout->isEnabled())
    {
      m_autoLayout->frameChanged();
    }
    else
    {
      // todo, scale subview if no layout
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

} // namespace VGG