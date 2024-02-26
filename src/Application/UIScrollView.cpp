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

#include "UIScrollView.hpp"

using namespace VGG;

bool UIScrollView::onEvent(UEvent event, void* userData)
{
  switch (event.type)
  {
    case VGG_MOUSEBUTTONDOWN:
    case VGG_MOUSEMOTION:
    case VGG_MOUSEBUTTONUP:

    case VGG_TOUCHDOWN:
    case VGG_TOUCHMOTION:
    case VGG_TOUCHUP:
      break;

    default:
      break;
  }

  return UIView::onEvent(event, userData);
}

void UIScrollView::setContentSize(UISize size)
{
  m_contentSize = size;
}

void UIScrollView::setContentOffset(UIPoint offset)
{
  m_contentOffset = offset;
}

bool UIScrollView::canScrollHorizontal()
{
  return m_contentSize.width > size().width;
}

bool UIScrollView::canScrollVertical()
{
  return m_contentSize.height > size().height;
}