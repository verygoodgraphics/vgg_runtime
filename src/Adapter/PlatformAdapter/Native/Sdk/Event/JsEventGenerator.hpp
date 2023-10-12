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
#pragma once

#include "Application/EventVisitor.hpp"
#include "UIEvent.hpp"

namespace VGG
{
namespace NodeAdapter
{

class JsEventGenerator : public EventVisitor
{
  std::string m_event_id;
  std::string m_script;

public:
  JsEventGenerator(std::string eventId)
    : m_event_id{ eventId }
  {
  }

  std::string getScript()
  {
    return m_script;
  }

  virtual void visit(VGG::KeyboardEvent* e) override
  {
    makeScript("VggKeyboardEvent");
  }

  virtual void visit(VGG::MouseEvent* e) override
  {
    makeScript("VggMouseEvent");
  }

  virtual void visit(VGG::TouchEvent* e) override
  {
    makeScript("VggTouchEvent");
  }

private:
  void makeScript(std::string_view event)
  {
    m_script = R"(
      var vggSdkAddon = process._linkedBinding('vgg_sdk_addon');
      var theVggEvent = new vggSdkAddon.)";
    m_script.append(event);
    m_script.append("(); theVggEvent.bindCppEvent(");
    m_script.append(m_event_id);
    m_script.append(");");
  }
};

} // namespace NodeAdapter
} // namespace VGG
