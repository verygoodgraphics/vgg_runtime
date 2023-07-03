#pragma once

#include "UIEvent.hpp"

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
    char key = m_event_ptr->key;
    return { key };
  };

  bool repeat() const
  {
    return m_event_ptr->repeat;
  }

  bool altkey() const
  {
    return m_event_ptr->altKey;
  }
  bool ctrlkey() const
  {
    return m_event_ptr->ctrlKey;
  }
  bool metakey() const
  {
    return m_event_ptr->metaKey;
  }
  bool shiftkey() const
  {
    return m_event_ptr->shiftKey;
  }
};

} // namespace NodeAdapter
} // namespace VGG