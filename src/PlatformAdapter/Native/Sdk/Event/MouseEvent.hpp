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
  static napi_value button(napi_env env, napi_callback_info info);

  // Method
  static napi_value getModifierState(napi_env env, napi_callback_info info);
};

} // namespace NodeAdapter
} // namespace VGG