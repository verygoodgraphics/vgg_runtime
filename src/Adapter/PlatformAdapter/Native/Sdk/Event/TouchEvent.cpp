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
#include "TouchEvent.hpp"

#include "PlatformAdapter/Native/Sdk/defines.hpp"

namespace VGG
{
namespace NodeAdapter
{

void TouchEvent::Init(napi_env env, napi_value exports)
{
  properties_type properties = {};

  auto base_properties{ base_type::properties() };
  properties.insert(properties.end(), base_properties.begin(), base_properties.end());

  napi_value cons;
  NODE_API_CALL_RETURN_VOID(env,
                            napi_define_class(env,
                                              "VggTouchEvent",
                                              -1,
                                              New,
                                              nullptr,
                                              properties.size(),
                                              properties.data(),
                                              &cons));

  NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &s_constructor));

  NODE_API_CALL_RETURN_VOID(env, napi_set_named_property(env, exports, "VggTouchEvent", cons));
}

} // namespace NodeAdapter
} // namespace VGG
