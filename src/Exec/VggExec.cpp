#include "VggExec.hpp"

bool VggExec::evalScript(const std::string& program)
{
  setEnv();
  return m_jsEngine->evalScript(program);
}

bool VggExec::evalModule(const std::string& program)
{
  setEnv();
  return m_jsEngine->evalModule(program);
}

void VggExec::setEnv()
{
  auto env = m_env->getEnv();
  // todo: set env, then eval
  // m_jsEngine->eval("setEnv");
}
