#ifndef NATIVE_EXEC_HPP
#define NATIVE_EXEC_HPP

#include "VggJSEngine.hpp"

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

  bool eval(std::string_view buffer, const char* filename, int flags);

  std::function<void(node::Environment*)> m_envDidLoad;
  std::function<const char*()> m_getInitScriptForEnv;

private:
  NativeExecImpl* pImpl;
};

#endif