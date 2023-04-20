#ifndef NATIVE_EXEC_HPP
#define NATIVE_EXEC_HPP

#include "VggJSEngine.hpp"

#include <functional>
#include <memory>
#include <thread>

namespace node
{
class Environment;
}

struct NativeExecImpl;

class NativeExec : public VggJSEngine
{
public:
  NativeExec();
  ~NativeExec();

  bool evalScript(const std::string& code);
  bool evalModule(const std::string& code);

private:
  std::shared_ptr<std::thread> m_thread;
  std::shared_ptr<NativeExecImpl> m_impl;

  std::function<void(const node::Environment*)> m_envDidLoad;
  std::function<const char*()> m_getInitScriptForEnv;

  void teardown();
};

#endif