#pragma once

#include "env-inl.h"
#include "node.h"
#include "util-inl.h"
#include "uv.h"

#include <assert.h>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_set>

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

struct NativeExecImpl;

struct NativeEvalTask
{
  std::string m_code;
  uv_async_t m_async_task;
  NativeExecImpl* m_exec_impl_ptr;

  void run();
};

class NativeExecImpl
{
public:
  bool schedule_eval(const std::string& code);
  int run_node(const int argc, const char** argv, std::shared_ptr<std::thread>& nodeThread);
  void stop_node_thread_safe();

  node::Environment* getNodeEnv();

private:
  friend NativeEvalTask;
  int eval(const std::string_view buffer);

  int node_main(const std::vector<std::string>& args);
  int run_node_instance(MultiIsolatePlatform* platform,
                        const std::vector<std::string>& args,
                        const std::vector<std::string>& exec_args);
  void run_task(NativeEvalTask* task);
  void erase_task(NativeEvalTask* task);
  bool check_state();

  enum ExecState
  {
    INIT,
    CANCELLED,
    RUNNING,
    DEAD,
  };
  ExecState m_state{ INIT };
  CommonEnvironmentSetup* m_setup;
  Isolate* m_isolate;
  Environment* m_env;
  uv_loop_t* m_loop;

  std::unordered_set<NativeEvalTask*> m_tasks;
  std::mutex m_tasks_mutex;

  std::mutex m_state_mutex;
  uv_timer_t m_keep_alive_timer;
  uv_async_t m_stop_timer_async;
};