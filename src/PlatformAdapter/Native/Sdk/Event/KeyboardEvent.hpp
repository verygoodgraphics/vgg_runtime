#pragma once

#include "UIEvent.hpp"

#include "js_native_api.h"

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
};

} // namespace NodeAdapter
} // namespace VGG