#ifndef __VGG_EXEC_H__
#define __VGG_EXEC_H__

#include "VggJSEngine.hpp"
#include "VggEnv.hpp"

#include <memory>
#include <string>

class VggExec
{
public:
  VggExec(std::shared_ptr<VggJSEngine> jsEngine, std::shared_ptr<VggEnv> env)
    : m_jsEngine(jsEngine)
    , m_env(env)
  {
  }
  ~VggExec() = default;

  bool eval(const std::string& program);

private:
  std::shared_ptr<VggJSEngine> m_jsEngine;
  std::shared_ptr<VggEnv> m_env;
};

#endif