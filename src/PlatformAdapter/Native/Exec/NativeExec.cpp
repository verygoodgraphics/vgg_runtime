#include "NativeExec.hpp"

#include "NativeExecImpl.hpp"
#include "StringHelper.hpp"
#include "Utils/Utils.hpp"

using namespace VGG;

NativeExec::NativeExec()
  : m_impl(new NativeExecImpl)
{
  const char* argv[] = {
    "", // first arg is program name, mock it
    "--expose-internals",
    "--experimental-network-imports",
  };
  m_impl->run_node(3, argv, m_thread, m_envDidLoad, m_getInitScriptForEnv);
}

NativeExec::~NativeExec()
{
  teardown();
  m_thread->join();
}

bool NativeExec::evalScript(const std::string& code)
{
  return m_impl->schedule_eval(code);
}

bool NativeExec::evalModule(const std::string& code)
{
  std::string wrapped_script(R"(
    var { evalModule } = require('internal/process/execution');
    var encoded_code = ')");
  wrapped_script.append(StringHelper::url_encode(code));
  wrapped_script.append("';\n");
  wrapped_script.append("evalModule(decodeURIComponent(encoded_code));");

  DEBUG("#NativeExec, evalModule: %s", wrapped_script.c_str());

  return m_impl->schedule_eval(wrapped_script);
}

void NativeExec::teardown()
{
  if (m_impl)
  {
    m_impl->stop_node_thread_safe();
  }
}
