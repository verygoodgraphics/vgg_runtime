#include "ResizeWindow.hpp"

namespace VGG
{

void ResizeWindow::onResize(Layout::Size newSize)
{
  m_layout->layout(newSize);
}

} // namespace VGG
