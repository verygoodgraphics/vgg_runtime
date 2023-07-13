#pragma once

#include "Application/EventVisitor.hpp"
#include "Application/UIEvent.hpp"
#include "UIEvent.hpp"

namespace VGG
{
namespace BrowserAdapter
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
      const { getVgg } = await import('https://s5.vgg.cool/vgg-sdk.esm.js');
      const vgg = await getVgg();
      var theVggEvent = new vgg.)";
    m_script.append(event);
    m_script.append("(); theVggEvent.bindCppEvent(");
    m_script.append(m_event_id);
    m_script.append(");");
  }
};

} // namespace BrowserAdapter
} // namespace VGG