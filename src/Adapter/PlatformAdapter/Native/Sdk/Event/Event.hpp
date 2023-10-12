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

#include "PlatformAdapter/Native/Sdk/AdapterHelper.hpp"

#include "Utility/Log.hpp"

#include <node_api.h>

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace VGG
{
namespace NodeAdapter
{

template<class EventAdapterType, class EventType>
class Event
{
  using child_type = EventAdapterType;
  using event_type = EventType;

  using map_key_type = int;
  inline static std::mutex m_map_mutex;
  inline static std::unordered_map<map_key_type, std::shared_ptr<event_type>> s_event_map;
  inline static map_key_type s_event_id{ 0 };

protected:
  inline static napi_ref s_constructor;

  napi_env m_env;
  napi_ref m_wrapper;
  std::shared_ptr<event_type> m_event_ptr;

public:
  using properties_type = std::vector<napi_property_descriptor>;

  static auto store(std::shared_ptr<event_type> event)
  {
    const std::lock_guard<std::mutex> lock(m_map_mutex);

    s_event_map[s_event_id] = event;
    return s_event_id++;
  }

  virtual ~Event()
  {
    napi_delete_reference(m_env, m_wrapper);
  }

  // template
  template<auto f>
  static napi_value stringMethod(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    child_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    auto result = std::invoke(f, wrapper);

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, result.data(), result.size(), &ret));

    return ret;
  }

  template<auto f>
  static napi_value intMethod(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    child_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    auto result = std::invoke(f, wrapper);

    napi_value ret;
    NODE_API_CALL(env, napi_create_int32(env, result, &ret));

    return ret;
  }

  template<auto f>
  static napi_value boolMethod(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    child_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    auto result = std::invoke(f, wrapper);

    napi_value ret;
    NODE_API_CALL(env, napi_get_boolean(env, result, &ret));

    return ret;
  }

protected:
  static napi_value New(napi_env env, napi_callback_info info)
  {
    napi_value new_target;
    NODE_API_CALL(env, napi_get_new_target(env, info, &new_target));
    bool is_constructor = (new_target != nullptr);

    size_t argc = 1;
    napi_value args[1];
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    if (is_constructor)
    {
      // Invoked as constructor: `new VggXxEvent()`
      auto instance = new child_type();
      instance->m_env = env;

      NODE_API_CALL(
        env,
        napi_wrap(env, _this, instance, child_type::Destructor, nullptr, &instance->m_wrapper));

      return _this;
    }

    // Invoked as plain function `VggXxEvent()`, turn into construct call.
    napi_value cons;
    NODE_API_CALL(env, napi_get_reference_value(env, s_constructor, &cons));

    napi_value instance;
    NODE_API_CALL(env, napi_new_instance(env, cons, 0, nullptr, &instance));

    return instance;
  }

  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint)
  {
  }

  // properties
  static napi_value target(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    child_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    ASSERT(wrapper->m_event_ptr);
    auto result = wrapper->m_event_ptr->target();

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, result.data(), result.size(), &ret));

    return ret;
  }

  static napi_value currentTarget(napi_env env, napi_callback_info info)
  {
    return nullptr;
  }

  static napi_value type(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    child_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    ASSERT(wrapper->m_event_ptr);
    auto result = wrapper->m_event_ptr->type();

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, result.data(), result.size(), &ret));

    return ret;
  }

  // method
  static napi_value preventDefault(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    child_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    ASSERT(wrapper->m_event_ptr);
    wrapper->m_event_ptr->preventDefault();

    return nullptr;
  }

  static napi_value stopImmediatePropagation(napi_env env, napi_callback_info info);
  static napi_value stopPropagation(napi_env env, napi_callback_info info);

  // vgg internal method
  static napi_value bindCppEvent(napi_env env, napi_callback_info info)
  {
    size_t argc = 1;
    napi_value args[1];
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));
    if (argc != 1)
    {
      napi_throw_error(env, nullptr, "Wrong number of arguments");
      return nullptr;
    }

    try
    {
      auto event_id = adapter::GetArgIntValue(env, args[0]);

      child_type* wrapper;
      NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

      wrapper->m_event_ptr = s_event_map[event_id];
      ASSERT(wrapper->m_event_ptr);

      const std::lock_guard<std::mutex> lock(m_map_mutex);
      s_event_map.erase(event_id);
    }
    catch (std::exception& e)
    {
      napi_throw_error(env, nullptr, e.what());
    }

    return nullptr;
  }

  static properties_type properties()
  {
    return { DECLARE_NODE_API_GETTER("target", target),
             DECLARE_NODE_API_GETTER("type", type),
             DECLARE_NODE_API_PROPERTY("preventDefault", preventDefault),
             DECLARE_NODE_API_PROPERTY("bindCppEvent", bindCppEvent) };
  }

private:
};

} // namespace NodeAdapter
} // namespace VGG
