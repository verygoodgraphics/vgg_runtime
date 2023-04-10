#ifndef NATIVE_EXEC_HPP
#define NATIVE_EXEC_HPP

#include "VggJSEngine.hpp"

#include <memory>

namespace node
{
class Environment;
}

struct NativeExecImpl;

class NativeExec : public VggJSEngine
{
public:
  // todo, remove thread
  NativeExec(std::shared_ptr<std::thread>& thread);
  ~NativeExec();

  bool evalScript(const std::string& code);
  bool evalModule(const std::string& code);

  std::function<void(node::Environment*)> m_envDidLoad;
  std::function<const char*()> m_getInitScriptForEnv;

private:
  std::shared_ptr<std::thread> m_thread;
  std::shared_ptr<NativeExecImpl> m_impl;

  void teardown();
};

#endif