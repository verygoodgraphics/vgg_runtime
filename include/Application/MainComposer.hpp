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

#include "Application/Controller.hpp"
#include "Application/Mouse.hpp"
#include "Application/PlatformComposer.hpp"
#include "Application/VggEnv.hpp"

#include "Domain/Daruma.hpp"

#include <memory>

namespace VGG
{

class Editor;
class Presenter;
class RunLoop;
class UIScrollView;
class UIView;

class MainComposer
{
  std::shared_ptr<VggEnv> m_env;

  std::shared_ptr<UIScrollView> m_view;
  std::shared_ptr<UIView>       m_editView;
  std::shared_ptr<Presenter>    m_presenter;
  std::shared_ptr<Editor>       m_editor;

  std::shared_ptr<RunLoop>    m_runLoop;
  std::shared_ptr<Controller> m_controller;

  std::shared_ptr<PlatformComposer> m_platformComposer;

public:
  MainComposer(PlatformComposer* platformComposer, std::shared_ptr<Mouse> mouse);

  ~MainComposer()
  {
    m_platformComposer->teardown();
  }

  auto env()
  {
    return m_env;
  }

  auto runLoop()
  {
    return m_runLoop;
  }

  auto controller()
  {
    return m_controller;
  }

  auto view()
  {
    return m_view;
  }

  void enableEdit(int top = 0, int right = 300, int bottom = 0, int left = 300);
};

} // namespace VGG
