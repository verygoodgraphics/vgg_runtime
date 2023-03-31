#include "NativeExec.hpp"

#include <assert.h>
#include <iostream>
#include <string>

#include "node.h"
#include "uv.h"

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

struct NativeExecImpl
{
  std::unique_ptr<MultiIsolatePlatform> platform_ = nullptr;
  std::unique_ptr<node::InitializationResult> result = nullptr;
  std::unique_ptr<CommonEnvironmentSetup> setup_ = nullptr;
  Environment* env = nullptr;
  Isolate* isolate = nullptr;

  int loadEnv(const char* script);
  int eval(std::string_view buffer);

  int setup(int argc,
            char** argv,
            std::function<void(node::Environment*)>& envHook,
            std::function<const char*()>& getScriptHook);
  void teardown();
};

NativeExec::NativeExec()
  : pImpl(new NativeExecImpl)
{
  const char* argv[] = { "" }; // first arg is program name, mock it
  pImpl->setup(1, const_cast<char**>(argv), m_envDidLoad, m_getInitScriptForEnv);
}

NativeExec::~NativeExec()
{
  pImpl->teardown();
  delete pImpl;
}

bool NativeExec::evalScript(const std::string& code)
{
  return 0 == pImpl->eval(code);
}

bool NativeExec::evalModule(const std::string& code)
{
  return 0 == pImpl->eval(code);
}

int NativeExecImpl::loadEnv(const char* script)
{
  int exit_code = 0;

  // todo: inject
  // link_vgg_sdk_addon(env);
  // const char* requrie_vgg_sdk_script =
  //   "const vggSdkAddon = process._linkedBinding('vgg_sdk_addon');"
  //   "globalThis.vggSdk = new vggSdkAddon.VggSdk()";
  {
    Locker locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Context::Scope context_scope(setup_->context());

    const char* setup_require_script =
      "const publicRequire = require('module').createRequire(process.cwd() + '/');"
      "globalThis.require = publicRequire;";
    MaybeLocal<Value> loadenv_ret = node::LoadEnvironment(env, setup_require_script);
    if (loadenv_ret.IsEmpty()) // There has been a JS exception.
      return 1;

    exit_code = node::SpinEventLoop(env).FromMaybe(1);
  }

  return exit_code;
}

int NativeExecImpl::eval(std::string_view buffer)
{
  Locker locker(isolate);
  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);
  auto context = setup_->context();
  Context::Scope context_scope(context);

  auto maybe_local_v8_string = v8::String::NewFromUtf8(isolate, buffer.data());
  if (maybe_local_v8_string.IsEmpty())
  {
    return -1;
  }

  auto local_v8_string = maybe_local_v8_string.ToLocalChecked();
  auto maybe_script = v8::Script::Compile(context, local_v8_string);
  if (maybe_script.IsEmpty())
  {
    return -1;
  }

  auto script = maybe_script.ToLocalChecked();
  auto script_result = script->Run(context);
  if (script_result.IsEmpty())
  {
    return -1;
  }

  return node::SpinEventLoop(env).FromMaybe(1);
}

int NativeExecImpl::setup(int argc,
                          char** argv,
                          std::function<void(node::Environment*)>& envHook,
                          std::function<const char*()>& getScriptHook)
{
  argv = uv_setup_args(argc, argv);
  std::vector<std::string> args(argv, argv + argc);
  result = node::InitializeOncePerProcess(
    args,
    { node::ProcessInitializationFlags::kNoInitializeV8,
      node::ProcessInitializationFlags::kNoInitializeNodeV8Platform });

  for (const std::string& error : result->errors())
    fprintf(stderr, "%s: %s\n", args[0].c_str(), error.c_str());
  if (result->early_return() != 0)
  {
    return result->exit_code();
  }

  platform_ = MultiIsolatePlatform::Create(4);
  V8::InitializePlatform(platform_.get());
  V8::Initialize();

  MultiIsolatePlatform* platform = platform_.get();
  const std::vector<std::string>& resultArgs = result->args();
  const std::vector<std::string>& exec_args = result->exec_args();

  std::vector<std::string> errors;
  setup_ = CommonEnvironmentSetup::Create(platform, &errors, resultArgs, exec_args);
  if (!setup_)
  {
    for (const std::string& err : errors)
      fprintf(stderr, "%s: %s\n", resultArgs[0].c_str(), err.c_str());
    return 1;
  }

  isolate = setup_->isolate();
  env = setup_->env();
  if (envHook)
  {
    envHook(env);
  }

  const char* script = getScriptHook ? getScriptHook() : "";
  return loadEnv(script);
}

void NativeExecImpl::teardown()
{
  node::Stop(env);
  setup_.reset();

  V8::Dispose();
  V8::DisposePlatform();

  node::TearDownOncePerProcess();
}