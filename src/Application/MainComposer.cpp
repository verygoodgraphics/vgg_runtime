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
#include "Application/MainComposer.hpp"
#include <string>
#include "Controller.hpp"
#include "Editor.hpp"
#include "PlatformComposer.hpp"
#include "Presenter.hpp"
#include "RunLoop.hpp"
#include "UIScrollView.hpp"
#include "UIView.hpp"
#include "VggEnv.hpp"
namespace VGG
{
class Mouse;
}

namespace VGG
{

MainComposer::MainComposer(PlatformComposer* platformComposer, std::shared_ptr<Mouse> mouse)
  : m_env{ new VggEnv }
  , m_view{ new UIScrollView }
  , m_presenter{ new Presenter{ mouse } }
  , m_editor{ new Editor{ m_view, mouse } }
  , m_runLoop{ RunLoop::sharedInstance() }
  , m_platformComposer{ platformComposer }
{
  VggEnv::set(m_env->getEnv(), m_env);

  m_presenter->setView(m_view);

  auto jsEngine = m_platformComposer->setup(m_env);
  m_controller.reset(new Controller{ m_env, m_runLoop, jsEngine, m_presenter, m_editor });

  m_env->setController(m_controller);
  m_env->setPresenter(m_presenter);
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

} // namespace VGG