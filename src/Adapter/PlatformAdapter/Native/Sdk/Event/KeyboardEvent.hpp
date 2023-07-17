#pragma once

#include "UIEvent.hpp"

#include "Log.h"

#include "node_api.h"

namespace VGG
{
namespace NodeAdapter
{

class KeyboardEvent : public UIEvent<KeyboardEvent, VGG::KeyboardEvent>
{
  using base_type = UIEvent<KeyboardEvent, VGG::KeyboardEvent>;

public:
  static void Init(napi_env env, napi_value exports);

private:
  static napi_value getModifierState(napi_env env, napi_callback_info info);

  std::string key()
  {
    ASSERT(m_event_ptr);
    char key = m_event_ptr->key;
    return { key };
  };

  bool repeat() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->repeat;
  }

  bool altkey() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->altKey;
  }
  bool ctrlkey() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->ctrlKey;
  }
  bool metakey() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->metaKey;
  }
  bool shiftkey() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->shiftKey;
  }
};

} // namespace NodeAdapter
} // namespace VGG
