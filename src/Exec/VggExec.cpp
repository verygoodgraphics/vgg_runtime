#include "VggExec.hpp"

bool VggExec::eval(const std::string& program)
{
  auto env = m_env->getEnv();
  // todo: set env, then eval
  // m_jsEngine->eval("setEnv");
  return m_jsEngine->eval(program);
}

// Private