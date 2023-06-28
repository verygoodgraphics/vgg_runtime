#pragma once

#include "js_native_api.h"

namespace VGG
{
namespace NodeAdapter
{

template<class T>
class Event
{
  using this_type = Event<T>;

protected:
  inline static napi_ref s_constructor;
  inline static std::shared_ptr<T> s_event_ptr; // todo, use map

  napi_env m_env;
  napi_ref m_wrapper;
  std::shared_ptr<T> m_event_ptr;

public:
  static void store(std::shared_ptr<T> event)
  {
    s_event_ptr = event;
  }

  virtual ~Event()
  {
    napi_delete_reference(m_env, m_wrapper);
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
      auto instance = new this_type();
      instance->m_env = env;

      NODE_API_CALL(
        env,
        napi_wrap(env, _this, instance, this_type::Destructor, nullptr, &instance->m_wrapper));

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

  static napi_value preventDefault(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    this_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    wrapper->m_event_ptr->preventDefault();

    return nullptr;
  }

  static napi_value stopImmediatePropagation(napi_env env, napi_callback_info info);
  static napi_value stopPropagation(napi_env env, napi_callback_info info);

  static napi_value bindCppEvent(napi_env env, napi_callback_info info)
  {
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    this_type* wrapper;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

    wrapper->m_event_ptr = s_event_ptr;

    return nullptr;
  }
};

} // namespace NodeAdapter
} // namespace VGG