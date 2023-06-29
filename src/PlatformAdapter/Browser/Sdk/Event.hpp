#pragma once

#include "Presenter/Event.hpp"

namespace VGG
{
namespace BrowserAdapter
{

class Event
{
  using event_type = VGG::Event;

  using map_key_type = int;
  inline static std::unordered_map<map_key_type, std::shared_ptr<event_type>> s_event_map;
  inline static map_key_type s_event_id{ 0 };

protected:
  std::shared_ptr<event_type> m_event_ptr;

  template<class T>
  std::shared_ptr<T> getEvent() const
  {
    return std::dynamic_pointer_cast<T>(m_event_ptr);
  }

public:
  virtual ~Event() = default;

  // staitc
  static auto store(std::shared_ptr<event_type> event)
  {
    s_event_map[s_event_id] = event;
    return std::to_string(s_event_id++);
  }

  // getter
  std::string target() const
  {
    return m_event_ptr->target();
  }

  std::string type() const
  {
    return m_event_ptr->type();
  }

  // method
  void preventDefault()
  {
    m_event_ptr->preventDefault();
  }

  // helper method
  void bindCppEvent(int eventId)
  {
    m_event_ptr = s_event_map[eventId];

    s_event_map.erase(eventId);
  }
};

} // namespace BrowserAdapter
} // namespace VGG