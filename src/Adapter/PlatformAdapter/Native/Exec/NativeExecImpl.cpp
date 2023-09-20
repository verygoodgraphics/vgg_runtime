#include "PlatformAdapter/Native/Exec/NativeExecImpl.hpp"

#include "Utility/Log.h"

#undef INFO
#define INFO(msg, ...)

#undef DEBUG
#define DEBUG(msg, ...)

constexpr int THREAD_POOL_SIZE = 4;

/*
 * NativeExecImpl
 */
bool NativeExecImpl::schedule_eval(const std::string& code)
{
  if (!check_state())
  {
    FAIL("#NativeExecImpl::schedule_eval, error state");
    return false;
  }

  NativeEvalTask* task = new NativeEvalTask();
  task->m_code = code;
  task->m_exec_impl_ptr = this;

  {
    const std::lock_guard<std::mutex> lock(m_tasks_mutex);
    m_tasks.push(task);
  }
  uv_async_send(&m_async_task);

  return true;
}

int NativeExecImpl::eval(std::string_view buffer)
{
  DEBUG("#NativeExecImpl::eval, enter");

  Locker locker(m_isolate);
  Isolate::Scope isolate_scope(m_isolate);
  HandleScope handle_scope(m_isolate);

  auto context = m_setup->context();
  Context::Scope context_scope(context);

  auto maybe_local_v8_string = v8::String::NewFromUtf8(m_isolate, buffer.data());
  if (maybe_local_v8_string.IsEmpty())
  {
    FAIL("#NativeExecImpl::eval, error, script is empty");
    return -1;
  }

  auto local_v8_string = maybe_local_v8_string.ToLocalChecked();
  auto maybe_script = v8::Script::Compile(context, local_v8_string);
  if (maybe_script.IsEmpty())
  {
    FAIL("#NativeExecImpl::eval, error, complie script error");
    std::cerr << "script is: " << buffer << std::endl;
    return -1;
  }

  DEBUG("#NativeExecImpl::eval, run");
  auto script = maybe_script.ToLocalChecked();
  auto script_result = script->Run(context);
  if (script_result.IsEmpty())
  {
    WARN("#NativeExecImpl::eval, run script return empty result");
    return -1;
  }

  DEBUG("#NativeExecImpl::eval, success");
  return 0;
}

int NativeExecImpl::run_node(const int argc,
                             const char** argv,
                             std::shared_ptr<std::thread>& nodeThread)
{
  uv_setup_args(argc, const_cast<char**>(argv));
  std::vector<std::string> args(argv, argv + argc);

  nodeThread.reset(new std::thread(
    [&, args]()
    {
      int ret = node_main(args);
      INFO("#node thread exit, ret = %d", ret);
    }));

  return 0;
}

int NativeExecImpl::node_main(const std::vector<std::string>& args)
{
  std::unique_ptr<node::InitializationResult> result = node::InitializeOncePerProcess(
    args,
    { node::ProcessInitializationFlags::kNoInitializeV8,
      node::ProcessInitializationFlags::kNoInitializeNodeV8Platform });

  for (const std::string& error : result->errors())
    fprintf(stderr, "%s: %s\n", args[0].c_str(), error.c_str());
  if (result->early_return() != 0)
  {
    return result->exit_code();
  }

  // Create a v8::Platform instance. `MultiIsolatePlatform::Create()` is a way
  // to create a v8::Platform instance that Node.js can use when creating
  // Worker threads. When no `MultiIsolatePlatform` instance is present,
  // Worker threads are disabled.
  std::unique_ptr<MultiIsolatePlatform> platform = MultiIsolatePlatform::Create(THREAD_POOL_SIZE);
  V8::InitializePlatform(platform.get());
  V8::Initialize();

  int ret = run_node_instance(platform.get(), result->args(), result->exec_args());

  V8::Dispose();
  V8::DisposePlatform();

  node::TearDownOncePerProcess();
  return ret;
}

node::Environment* NativeExecImpl::getNodeEnv()
{
  return check_state() ? m_env : nullptr;
}

bool NativeExecImpl::check_state()
{
  if (m_state == DEAD)
  {
    FAIL("#NativeExecImpl::check_state, dead state");
    return false;
  }

  // Wait node thread ready. 'm_state' will be updated in run_node_instance
  while (m_state != RUNNING)
  {
    // volatile m_state will prevent this loop from being optimized away in release build.
  }

  return true;
}

int NativeExecImpl::run_node_instance(MultiIsolatePlatform* platform,
                                      const std::vector<std::string>& args,
                                      const std::vector<std::string>& exec_args)
{
  int exit_code = 0;

  std::vector<std::string> errors;
  std::unique_ptr<CommonEnvironmentSetup> setup =
    CommonEnvironmentSetup::Create(platform, &errors, args, exec_args);
  if (!setup)
  {
    for (const std::string& err : errors)
      fprintf(stderr, "%s: %s\n", args[0].c_str(), err.c_str());
    return 1;
  }

  Isolate* isolate = setup->isolate();
  Environment* env = setup->env();

  // save ptr ref to member filed
  m_setup = setup.get();
  m_isolate = isolate;
  m_env = env;
  m_loop = setup->event_loop();

  {
    Locker locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Context::Scope context_scope(setup->context());

    MaybeLocal<Value> loadenv_ret = node::LoadEnvironment(
      env,
      "globalThis.require = require('module').createRequire(process.cwd() + '/');");

    if (loadenv_ret.IsEmpty()) // There has been a JS exception.
      return 1;

    if (m_state != CANCELLED)
    {
      INFO("#exec, init keep alive timer");
      uv_timer_init(setup->event_loop(), &m_keep_alive_timer);
      uv_timer_start(
        &m_keep_alive_timer,
        [](uv_timer_t* timer) {},
        std::numeric_limits<uint64_t>::max(),
        0);

      init_uv_async_task();

      const std::lock_guard<std::mutex> lock(m_state_mutex);
      m_state = RUNNING;

      INFO("#exec, node run");
      exit_code = node::SpinEventLoop(env).FromMaybe(1);

      deinit_uv_async_task();
    }

    stop_node();
  }

  m_state = DEAD;
  INFO("#exec, node exit");

  return exit_code;
}

void NativeExecImpl::stop_node()
{
  if (m_env)
  {
    node::Stop(m_env);
    m_env = nullptr;
  }
}

void NativeExecImpl::notify_node_thread_to_stop()
{
  if (m_state != RUNNING)
  {
    const std::lock_guard<std::mutex> lock(m_state_mutex);
    m_state = CANCELLED;

    INFO("#exec, cancel node");
    return;
  }

  uv_async_send(&m_stop_timer_async);
}

void NativeExecImpl::init_uv_async_task()
{
  uv_async_init(m_loop,
                &m_async_task,
                [](uv_async_t* async)
                {
                  // Run task in uv event loop.
                  auto self_ptr = static_cast<NativeExecImpl*>(async->data);
                  self_ptr->run_task();
                });
  m_async_task.data = static_cast<void*>(this);

  uv_async_init(m_loop,
                &m_stop_timer_async,
                [](uv_async_t* async)
                {
                  DEBUG("#exec, stop keep alive timer");
                  auto self_ptr = static_cast<NativeExecImpl*>(async->data);
                  uv_timer_stop(&self_ptr->m_keep_alive_timer);
                  uv_close(reinterpret_cast<uv_handle_t*>(&self_ptr->m_keep_alive_timer), nullptr);
                  uv_close(reinterpret_cast<uv_handle_t*>(&self_ptr->m_stop_timer_async), nullptr);
                });
  m_stop_timer_async.data = static_cast<void*>(this);
}

void NativeExecImpl::deinit_uv_async_task()
{
  uv_close(reinterpret_cast<uv_handle_t*>(&m_async_task), nullptr);
}

void NativeExecImpl::run_task()
{
  while (!m_tasks.empty())
  {
    auto task = m_tasks.front();
    {
      const std::lock_guard<std::mutex> lock(m_tasks_mutex);
      m_tasks.pop();
    }

    DEBUG("#evalScript, before eval");
    int ret = eval(task->m_code);
    DEBUG("#evalScript, after eval, ret = %d", ret);
  }
}
