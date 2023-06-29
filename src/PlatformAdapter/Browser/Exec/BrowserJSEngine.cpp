#include "PlatformAdapter/Browser/Exec/BrowserJSEngine.hpp"
#include "PlatformAdapter/Browser/Sdk/Event.hpp"
#include "PlatformAdapter/Browser/Sdk/JsEventGenerator.hpp"
#include "PlatformAdapter/Helper/StringHelper.hpp"

#include <emscripten/emscripten.h>

using namespace VGG;

bool BrowserJSEngine::evalScript(const std::string& code)
{
  emscripten_run_script(code.c_str());
  return true;
}

bool BrowserJSEngine::evalModule(const std::string& code)
{
  auto unique_code{ code + ";" + std::to_string(m_evalTimes++) + ";" };

  m_moduleWrapper.erase();

  m_moduleWrapper.append("const dataUri = ");
  m_moduleWrapper.append(StringHelper::encode_script_to_data_uri(unique_code));
  m_moduleWrapper.append("; import(dataUri);");

  emscripten_run_script(m_moduleWrapper.c_str());
  return true;
}

bool BrowserJSEngine::evalModule(const std::string& code, VGG::EventPtr event)
{
  auto event_id{ BrowserAdapter::Event::store(event) };
  BrowserAdapter::JsEventGenerator event_generator{ event_id };
  event->accept(&event_generator);

  // todo, import handleEvent need not to be unique;
  //  import call should be unique, so it call be evaluated multiple times

  auto unique_code{ code + ";" + std::to_string(m_evalTimes++) + ";" };

  std::string call_imported_function{ event_generator.getScript() };
  call_imported_function.append("const dataUri = ");
  call_imported_function.append(StringHelper::encode_script_to_data_uri(unique_code));
  call_imported_function.append(
    "; const { default: handleEvent } = await import(dataUri); handleEvent(theVggEvent);");

  m_moduleWrapper.erase();
  m_moduleWrapper.append("const dataUri = ");
  m_moduleWrapper.append(StringHelper::encode_script_to_data_uri(call_imported_function));
  m_moduleWrapper.append("; import(dataUri);");

  emscripten_run_script(m_moduleWrapper.c_str());

  return true;
}