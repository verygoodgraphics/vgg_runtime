#include "View.hpp"

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"

namespace VGG
{

std::shared_ptr<Layout::Internal::AutoLayout> LayoutView::autoLayout() const
{
  return m_autoLayout;
}

std::shared_ptr<Layout::Internal::AutoLayout> LayoutView::createAutoLayout()
{
  m_autoLayout.reset(new Layout::Internal::AutoLayout);
  m_autoLayout->view = weak_from_this();

  return m_autoLayout;
}

std::shared_ptr<LayoutView> LayoutView::autoLayoutContainer()
{
  return m_parent.lock();
}

void LayoutView::applyLayout()
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

void LayoutView::setNeedLayout()
{
  m_needsLayout = true;
}

void LayoutView::layoutIfNeeded()
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

const Layout::Rect& LayoutView::frame() const
{
  return m_frame;
}
void LayoutView::setFrame(const Layout::Rect& frame)
{
  if (m_frame != frame)
  {
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

    // todo, scale subview if no layout

    m_autoLayout->frameChanged();
  }
}

void LayoutView::setViewModel(JsonDocumentPtr viewModel)
{
  m_viewModel = viewModel;
}

void LayoutView::configureAutoLayout()
{
  if (m_autoLayout)
  {
    m_autoLayout->configure();
  }
}

} // namespace VGG