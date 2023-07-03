#ifndef NATIVE_EXEC_HPP
#define NATIVE_EXEC_HPP

#include "Exec/VggJSEngine.hpp"

#include <functional>
#include <memory>
#include <thread>

namespace node
{
class Environment;
}

struct NativeExecImpl;

class NativeExec final : public VggJSEngine
{
public:
  NativeExec();
  ~NativeExec();

  bool evalScript(const std::string& code) override;
  bool evalModule(const std::string& code, VGG::EventPtr event) override;

  bool evalModule(const std::string& code);

  using InjectFn = std::function<void(node::Environment*)>;
  bool inject(InjectFn fn);

private:
  std::shared_ptr<std::thread> m_thread;
  std::shared_ptr<NativeExecImpl> m_impl;

  void teardown();
};

#endif