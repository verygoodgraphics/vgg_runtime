#pragma once

#include "Presenter/EventVisitor.hpp"
#include "Presenter/UIEvent.hpp"
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

  std::string getScirpt()
  {
    return m_script;
  }

  virtual void visit(KeyboardEvent* e)
  {
    makeScript("VggKeyboardEvent");
  }

  virtual void visit(MouseEvent* e)
  {
    makeScript("VggMouseEvent");
  }

  virtual void visit(TouchEvent* e)
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