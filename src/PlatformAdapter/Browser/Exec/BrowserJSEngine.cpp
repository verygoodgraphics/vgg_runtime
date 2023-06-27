#include "PlatformAdapter/Browser/Exec/BrowserJSEngine.hpp"
#include "PlatformAdapter/Browser/Sdk/MouseEventWrapper.hpp"
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
  MouseEventWrapper::store(event); // todo fixme

  auto unique_code{ code + ";" + std::to_string(m_evalTimes++) + ";" };
  m_moduleWrapper.erase();

  std::string call_imported_function(R"(
    const { getVgg } = await import('https://s5.vgg.cool/vgg-sdk.esm.js');
    const vgg = await getVgg();
    var theVggEvent = new vgg.VggMouseEvent();
    theVggEvent.bindCppEvent(); // todo, bind the corresponding event
  )");
  call_imported_function.append("const dataUri = ");
  call_imported_function.append(StringHelper::encode_script_to_data_uri(unique_code));
  call_imported_function.append(
    "; const { default: handleEvent } = await import(dataUri); handleEvent(theVggEvent);");

  m_moduleWrapper.append("const dataUri = ");
  m_moduleWrapper.append(StringHelper::encode_script_to_data_uri(call_imported_function));
  m_moduleWrapper.append("; import(dataUri);");

  emscripten_run_script(m_moduleWrapper.c_str());

  return true;
}