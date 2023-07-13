#pragma once

#include "UIEvent.hpp"

#include "node_api.h"

namespace VGG
{
namespace NodeAdapter
{

class TouchEvent : public UIEvent<TouchEvent, VGG::TouchEvent>
{
  using base_type = UIEvent<TouchEvent, VGG::TouchEvent>;

public:
  static void Init(napi_env env, napi_value exports);

private:
};

} // namespace NodeAdapter
} // namespace VGG