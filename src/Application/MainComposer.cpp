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
#include "Application/MainComposer.hpp"
#include "Async.hpp"

#include "Editor.hpp"
#include "Presenter.hpp"
#include "RunLoop.hpp"
#include "UIView.hpp"

using namespace VGG;

MainComposer::MainComposer(PlatformComposer* platformComposer, std::shared_ptr<Mouse> mouse)
  : m_view{ new UIView }
  , m_presenter{ new Presenter{ mouse } }
  , m_editor{ new Editor{ m_view, mouse } }
  , m_runLoop{ new RunLoop }
  , m_platformComposer{ platformComposer }
{
  m_presenter->setView(m_view);

  auto jsEngine = m_platformComposer->setup();
  m_controller.reset(new Controller{ m_runLoop, jsEngine, m_presenter, m_editor });
}

void MainComposer::enableEdit(int top, int right, int bottom, int left)
{
  m_editView.reset(new UIView);
  m_presenter->setEditView(m_editView);

  if (m_view)
  {
    m_view->becomeEditorWithSidebar(top, right, bottom, left);
    m_view->addSubview(m_editView);
  }
}
