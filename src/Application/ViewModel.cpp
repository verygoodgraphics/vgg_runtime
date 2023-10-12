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
#include "ViewModel.hpp"

#include "Domain/Layout/Layout.hpp"
#include "Domain/RawJsonDocument.hpp"
#include "Utility/Log.hpp"

using namespace VGG;

std::shared_ptr<LayoutNode> ViewModel::layoutTree() const
{
  auto sharedLayout = layout.lock();
  ASSERT(sharedLayout);
  if (sharedLayout)
  {
    return sharedLayout->layoutTree();
  }

  return {};
}

JsonDocumentPtr ViewModel::designDoc() const
{
  auto sharedLayout = layout.lock();
  ASSERT(sharedLayout);
  if (sharedLayout)
  {
    return sharedLayout->displayDesignDoc();
  }

  return JsonDocumentPtr(new RawJsonDocument{});
}
