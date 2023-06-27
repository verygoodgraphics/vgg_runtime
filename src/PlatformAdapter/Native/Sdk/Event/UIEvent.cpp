#include "UIEvent.hpp"

#include "PlatformAdapter/Native/Sdk/defines.hpp"

namespace VGG
{
namespace NodeAdapter
{

napi_ref UIEvent::sm_constructor;
VGG::EventPtr UIEvent::tmp_event_ptr;

UIEvent::~UIEvent()
{
  napi_delete_reference(m_env, m_wrapper);
}

void UIEvent::Destructor(napi_env env, void* nativeObject, void*)
{
}

void UIEvent::Init(napi_env env, napi_value exports)
{
  napi_property_descriptor properties[] = {
    DECLARE_NODE_API_PROPERTY("preventDefault", preventDefault),
    DECLARE_NODE_API_PROPERTY("bindCppEvent", bindCppEvent)
  };

  napi_value cons;
  NODE_API_CALL_RETURN_VOID(env,
                            napi_define_class(env,
                                              "VggUIEvent",
                                              -1,
                                              New,
                                              nullptr,
                                              sizeof(properties) / sizeof(napi_property_descriptor),
                                              properties,
                                              &cons));

  NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &sm_constructor));

  NODE_API_CALL_RETURN_VOID(env, napi_set_named_property(env, exports, "VggUIEvent", cons));
}

napi_value UIEvent::New(napi_env env, napi_callback_info info)
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
    // Invoked as constructor: `new VggUIEvent()`
    UIEvent* instance = new UIEvent();
    instance->m_env = env;

    NODE_API_CALL(
      env,
      napi_wrap(env, _this, instance, UIEvent::Destructor, nullptr, &instance->m_wrapper));

    return _this;
  }

  // Invoked as plain function `VggUIEvent()`, turn into construct call.
  napi_value cons;
  NODE_API_CALL(env, napi_get_reference_value(env, sm_constructor, &cons));

  napi_value instance;
  NODE_API_CALL(env, napi_new_instance(env, cons, 0, nullptr, &instance));

  return instance;
}

napi_value UIEvent::preventDefault(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

  UIEvent* wrapper;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

  wrapper->m_event_ptr->preventDefault();

  return nullptr;
}

napi_value UIEvent::bindCppEvent(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

  UIEvent* wrapper;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&wrapper)));

  wrapper->m_event_ptr = tmp_event_ptr;

  return nullptr;
}

} // namespace NodeAdapter
} // namespace VGG