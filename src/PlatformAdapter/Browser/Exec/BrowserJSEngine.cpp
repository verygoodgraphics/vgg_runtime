#include "PlatformAdapter/Browser/Exec/BrowserJSEngine.hpp"
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
  m_moduleWrapper.erase();

  m_moduleWrapper.append("const dataUri = ");
  m_moduleWrapper.append(StringHelper::encode_script_to_data_uri(code));
  m_moduleWrapper.append("; import(dataUri);");

  emscripten_run_script(m_moduleWrapper.c_str());
  return true;
}
