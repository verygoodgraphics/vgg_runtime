#include "NativeExec.hpp"

#include "NativeExecImpl.hpp"
#include "StringHelper.hpp"
#include "Utils/Utils.hpp"

using namespace VGG;

NativeExec::NativeExec(std::shared_ptr<std::thread>& thread)
  : m_impl(new NativeExecImpl)
{
  // { // test http import
  //   auto code = R"(
  //   console.log('#argment script');
  // )";

  //   const char* argv[] = { "", // first arg is program name, mock it
  //                          "--expose-internals",
  //                          "--experimental-network-imports",
  //                          code };
  //   m_impl->run_node(4, const_cast<char**>(argv), m_envDidLoad, m_getInitScriptForEnv, thread);
  //   return; // debug
  // }

  // auto code = R"(
  //   const { evalModule } = require('internal/process/execution');
  //   const code = 'import("http://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{
  //   console.log("#theModule is: ", theModule); })';

  //   console.log('#before evalModule,  evalModule is: ', evalModule);
  //   evalModule(code);
  //   console.log('#after evalModule, evalModule is: ', evalModule);
  // )";

  // todo: import vgg https url and use default loader
  const char* argv[] = { "", // first arg is program name, mock it
                         "--expose-internals",
                         "--experimental-loader",
                         "./testDataDir/js/http-loader.mjs" };
  m_impl->run_node(4, const_cast<char**>(argv), m_envDidLoad, m_getInitScriptForEnv, m_thread);
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

    // m_impl = nullptr; // todo
  }
}
