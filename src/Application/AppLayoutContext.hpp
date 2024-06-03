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
#include "Application/AttrBridge.hpp"
#include "Domain/Layout/LayoutContext.hpp"

namespace VGG
{

class AppLayoutContext : public LayoutContext
{
public:
  AppLayoutContext(std::shared_ptr<AttrBridge> layerBridge);

  Layout::Size nodeSize(LayoutNode* node) const override;

  void didUpdateBounds(LayoutNode* node) override;
  void didUpdateMatrix(LayoutNode* node) override;
  void didUpdateContourPoints(LayoutNode* node) override;

private:
  std::shared_ptr<AttrBridge> m_layerBridge;
};

} // namespace VGG