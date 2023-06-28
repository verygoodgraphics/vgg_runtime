#pragma once

#include "UIEvent.hpp"

#include "js_native_api.h"

namespace VGG
{
namespace NodeAdapter
{

class MouseEvent : public BasicEvent<VGG::MouseEvent>
{
public:
  static void Init(napi_env env, napi_value exports);
  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

  ~MouseEvent();

private:
  static napi_value getModifierState(napi_env env, napi_callback_info info);
};

} // namespace NodeAdapter
} // namespace VGG