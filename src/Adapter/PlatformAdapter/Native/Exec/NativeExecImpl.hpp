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
#pragma once

#include <env-inl.h>
#include <node.h>
#include <util-inl.h>
#include <uv.h>

#include <assert.h>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#ifndef _MSC_VER
#include <unistd.h>
#endif

using node::CommonEnvironmentSetup;
using node::Environment;
using node::MultiIsolatePlatform;
using v8::Context;
using v8::HandleScope;
using v8::Isolate;
using v8::Locker;
using v8::MaybeLocal;
using v8::V8;
using v8::Value;

namespace VGG
{

class NativeExecImpl;

struct NativeEvalTask
{
  std::string     m_code;
  NativeExecImpl* m_exec_impl_ptr;
};

class NativeExecImpl
{
public:
  bool schedule_eval(const std::string& code);
  int  run_node(const int argc, const char** argv, std::shared_ptr<std::thread>& nodeThread);
  void notify_node_thread_to_stop();
  void stop_node();

  node::Environment* getNodeEnv();

private:
  int eval(const std::string_view buffer);

  int node_main(const std::vector<std::string>& args);
  int run_node_instance(
    MultiIsolatePlatform*           platform,
    const std::vector<std::string>& args,
    const std::vector<std::string>& exec_args);
  void run_task();
  bool check_state();

  void init_uv_async_task();
  void deinit_uv_async_task();

  enum ExecState
  {
    INIT,
    CANCELLED,
    RUNNING,
    DEAD,
  };
  volatile ExecState      m_state{ INIT };
  CommonEnvironmentSetup* m_setup;
  Isolate*                m_isolate;
  Environment*            m_env;
  uv_loop_t*              m_loop;

  std::queue<NativeEvalTask*> m_tasks;
  std::mutex                  m_tasks_mutex;

  std::mutex m_state_mutex;
  uv_timer_t m_keep_alive_timer;
  uv_async_t m_stop_timer_async;
  uv_async_t m_async_task;
};

} // namespace VGG