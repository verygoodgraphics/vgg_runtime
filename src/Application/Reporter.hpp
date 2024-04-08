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

#include "Editor.hpp"
#include "UIEvent.hpp"

#include "Domain/IVggEnv.hpp"

#include <nlohmann/json.hpp>

#include <memory>

namespace VGG
{

class VggExec;

class Reporter : public Editor::Listener
{
  std::weak_ptr<IVggEnv> m_env;
  std::weak_ptr<VggExec> m_jsEngine;

public:
  Reporter(std::weak_ptr<IVggEnv> env, std::weak_ptr<VggExec> jsEngine)
    : m_env{ env }
    , m_jsEngine{ jsEngine }
  {
  }

  void onSelectNode(std::weak_ptr<LayoutNode> node) override;
  void onFirstRender();
  void onEvent(UIEventPtr evt);

private:
  void sendEventToJs(const nlohmann::json& event);
};

} // namespace VGG
