#include "MouseEvent.hpp"

#include "PlatformAdapter/Native/Sdk/defines.hpp"

namespace VGG
{
namespace NodeAdapter
{

void MouseEvent::Init(napi_env env, napi_value exports)
{
  properties_type properties = {
    DECLARE_NODE_API_GETTER("altkey", &boolMethod<&MouseEvent::altkey>),
    DECLARE_NODE_API_GETTER("button", &intMethod<&MouseEvent::button>),
    DECLARE_NODE_API_GETTER("clientX", &intMethod<&MouseEvent::x>),
    DECLARE_NODE_API_GETTER("clientY", &intMethod<&MouseEvent::y>),
    DECLARE_NODE_API_GETTER("ctrlkey", &boolMethod<&MouseEvent::ctrlkey>),
    DECLARE_NODE_API_GETTER("metakey", &boolMethod<&MouseEvent::metakey>),
    DECLARE_NODE_API_GETTER("movementX", &intMethod<&MouseEvent::movementX>),
    DECLARE_NODE_API_GETTER("movementY", &intMethod<&MouseEvent::movementY>),
    DECLARE_NODE_API_GETTER("shiftkey", &boolMethod<&MouseEvent::shiftkey>),
    DECLARE_NODE_API_GETTER("x", &intMethod<&MouseEvent::x>),
    DECLARE_NODE_API_GETTER("y", &intMethod<&MouseEvent::y>),
    DECLARE_NODE_API_PROPERTY("getModifierState", getModifierState),
  };

  auto base_properties{ base_type::properties() };
  properties.insert(properties.end(), base_properties.begin(), base_properties.end());

  napi_value cons;
  NODE_API_CALL_RETURN_VOID(env,
                            napi_define_class(env,
                                              "VggMouseEvent",
                                              -1,
                                              New,
                                              nullptr,
                                              properties.size(),
                                              properties.data(),
                                              &cons));

  NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &s_constructor));

  NODE_API_CALL_RETURN_VOID(env, napi_set_named_property(env, exports, "VggMouseEvent", cons));
}

// Getter

// Method
napi_value MouseEvent::getModifierState(napi_env env, napi_callback_info info)
{
  return nullptr;
}

} // namespace NodeAdapter
} // namespace VGG