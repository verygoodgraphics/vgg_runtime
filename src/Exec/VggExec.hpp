#ifndef __VGG_EXEC_H__
#define __VGG_EXEC_H__

#include "VggJSEngine.hpp"
#include "IVggEnv.hpp"

#include <memory>
#include <string>

class VggExec
{
public:
  VggExec(std::shared_ptr<VggJSEngine> jsEngine, std::shared_ptr<IVggEnv> env)
    : m_jsEngine(jsEngine)
    , m_env(env)
  {
  }
  ~VggExec() = default;

  bool evalScript(const std::string& srcipt);
  bool evalModule(const std::string& module);

private:
  std::shared_ptr<VggJSEngine> m_jsEngine;
  std::shared_ptr<IVggEnv> m_env;

  void setEnv();
};

#endif