#include "MouseEvent.hpp"

#include "PlatformAdapter/Native/Sdk/defines.hpp"

namespace VGG
{
namespace NodeAdapter
{

void MouseEvent::Init(napi_env env, napi_value exports)
{
  napi_property_descriptor properties[] = {
    DECLARE_NODE_API_PROPERTY("preventDefault", preventDefault),
    DECLARE_NODE_API_PROPERTY("getModifierState", getModifierState),
    DECLARE_NODE_API_PROPERTY("bindCppEvent", bindCppEvent)
  };

  napi_value cons;
  NODE_API_CALL_RETURN_VOID(env,
                            napi_define_class(env,
                                              "VggMouseEvent",
                                              -1,
                                              New,
                                              nullptr,
                                              sizeof(properties) / sizeof(napi_property_descriptor),
                                              properties,
                                              &cons));

  NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &s_constructor));

  NODE_API_CALL_RETURN_VOID(env, napi_set_named_property(env, exports, "VggMouseEvent", cons));
}

napi_value MouseEvent::getModifierState(napi_env env, napi_callback_info info)
{
  return nullptr;
}

} // namespace NodeAdapter
} // namespace VGG