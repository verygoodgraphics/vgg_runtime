/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "NativeExec.hpp"

#include "PlatformAdapter/Native/Exec/NativeExecImpl.hpp"
#include "PlatformAdapter/Native/Sdk/Event/EventStore.hpp"
#include "PlatformAdapter/Native/Sdk/Event/JsEventGenerator.hpp"
#include "PlatformAdapter/Native/Sdk/Event/UIEvent.hpp"
#include "PlatformAdapter/Helper/StringHelper.hpp"
#include "Utility/Log.hpp"

using namespace VGG;

std::shared_ptr<NativeExec> NativeExec::sharedInstance()
{
  static auto s_sharedInstance = std::shared_ptr<NativeExec>(new NativeExec);
  return s_sharedInstance;
}

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

bool NativeExec::evalModule(
  const std::string&       code,
  VGG::EventPtr            event,
  std::shared_ptr<IVggEnv> env)
{
  NodeAdapter::EventStore       event_store{ event };
  NodeAdapter::JsEventGenerator event_generator{ event_store.eventId() };
  event->accept(&event_generator);

  std::string wrapped_script{ R"(
    var { evalModule } = require('internal/process/execution');
    var encoded_code = ')" };

  std::string call_imported_function{ event_generator.getScript() };
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
