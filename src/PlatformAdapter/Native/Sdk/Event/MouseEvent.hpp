#pragma once

#include "UIEvent.hpp"

#include "node_api.h"

namespace VGG
{
namespace NodeAdapter
{

class MouseEvent : public UIEvent<MouseEvent, VGG::MouseEvent>
{
  using base_type = UIEvent<MouseEvent, VGG::MouseEvent>;

public:
  static void Init(napi_env env, napi_value exports);

private:
  // Getter

  // Method
  static napi_value getModifierState(napi_env env, napi_callback_info info);

  // getter
  int button() const
  {
    return m_event_ptr->button;
  }

  int x() const
  {
    return m_event_ptr->x;
  }
  int y() const
  {
    return m_event_ptr->y;
  }

  int movementX() const
  {
    return m_event_ptr->movementX;
  }
  int movementY() const
  {
    return m_event_ptr->movementY;
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