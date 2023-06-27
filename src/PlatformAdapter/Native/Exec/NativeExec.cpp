#include "PlatformAdapter/Native/Exec/NativeExec.hpp"

#include "PlatformAdapter/Native/Exec/NativeExecImpl.hpp"
#include "PlatformAdapter/Native/Sdk/Event/UIEvent.hpp"
#include "PlatformAdapter/Helper/StringHelper.hpp"
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
  m_impl->run_node(3, argv, m_thread);
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

  return m_impl->schedule_eval(wrapped_script);
}

bool NativeExec::evalModule(const std::string& code, VGG::EventPtr event)
{
  NodeAdapter::UIEvent::store(event); // todo fixme

  std::string wrapped_script(R"(
    var vggSdkAddon = process._linkedBinding('vgg_sdk_addon');
    var theVggEvent = new vggSdkAddon.VggUIEvent();
    theVggEvent.bindCppEvent(); // todo, bind the corresponding event

    var { evalModule } = require('internal/process/execution');
    var encoded_code = ')");

  std::string call_imported_function;
  call_imported_function.append("const dataUri = ");
  call_imported_function.append(StringHelper::encode_script_to_data_uri(code));
  call_imported_function.append(
    "; const { default: handleEvent } = await import(dataUri); handleEvent(theVggEvent);");
  wrapped_script.append(StringHelper::url_encode(call_imported_function));

  wrapped_script.append("'; evalModule(decodeURIComponent(encoded_code));");

  return m_impl->schedule_eval(wrapped_script);
}

bool NativeExec::inject(InjectFn fn)
{
  auto env = m_impl->getNodeEnv();
  if (env)
  {
    fn(env);
    return true;
  }
  return false;
}

void NativeExec::teardown()
{
  m_impl->notify_node_thread_to_stop();
  m_impl->stop_node();
}
