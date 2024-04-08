/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "UIEvent.hpp"

#include "Utility/Log.hpp"

#include <node_api.h>

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
    ASSERT(m_event_ptr);
    return m_event_ptr->button;
  }

  int x() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->x;
  }
  int y() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->y;
  }

  int movementX() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->movementX;
  }
  int movementY() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->movementY;
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
