/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#pragma once

#include <memory>
#include <string>
#include "Domain/Daruma.hpp"
#include "Domain/Loader.hpp"
#include "Domain/Model/Element.hpp"
namespace VGG
{
class LayoutNode;
namespace Layout
{
class Layout;
}
} // namespace VGG

namespace VGG
{

struct ViewModel
{
  std::weak_ptr<Daruma>         model;
  std::weak_ptr<Layout::Layout> layout;

  std::shared_ptr<LayoutNode>             layoutTree() const;
  std::shared_ptr<Domain::DesignDocument> designDoc() const;

  VGG::Model::Loader::ResourcesType resources() const
  {
    if (auto sharedModel = model.lock())
    {
      return sharedModel->resources();
    }
    else
    {
      return {};
    }
  }
};

} // namespace VGG
