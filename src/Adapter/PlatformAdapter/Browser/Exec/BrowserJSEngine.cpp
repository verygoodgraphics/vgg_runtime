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
#include "BrowserJSEngine.hpp"
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
  return evalScript(code);
}

bool BrowserJSEngine::evalModule(const std::string& code, VGG::EventPtr event)
{
  auto event_id{ BrowserAdapter::Event::store(event) };
  BrowserAdapter::JsEventGenerator event_generator{ event_id };
  event->accept(&event_generator);

  std::string call_imported_function{ event_generator.getScript() };
  call_imported_function.append("const innerDataUri = ");
  call_imported_function.append(StringHelper::encode_script_to_data_uri(code));
  call_imported_function.append(
    "; const { default: handleEvent } = await import(innerDataUri); handleEvent(theVggEvent);");

  m_moduleWrapper.erase();
  m_moduleWrapper.append("const outerDataUri = ");
  m_moduleWrapper.append(StringHelper::encode_script_to_data_uri(call_imported_function));
  // The event_id is different, so the outerDataUri is also different.
  // Then each import will excute outerDataUri
  m_moduleWrapper.append("; import(outerDataUri);");

  emscripten_run_script(m_moduleWrapper.c_str());

  return true;
}
