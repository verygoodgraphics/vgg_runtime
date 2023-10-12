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
#include <node.h>
#include <node_api.h>
#include <util.h>

#include "VggSdkAddon.hpp"
#include "PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.hpp"
#include "PlatformAdapter/Native/Sdk/Event/KeyboardEvent.hpp"
#include "PlatformAdapter/Native/Sdk/Event/MouseEvent.hpp"
#include "PlatformAdapter/Native/Sdk/Event/TouchEvent.hpp"
#include "PlatformAdapter/Native/Sdk/Event/UIEvent.hpp"

napi_value InitializeLocalNapiBinding(napi_env env, napi_value exports)
{
  VggSdkNodeAdapter::Init(env, exports);
  VGG::NodeAdapter::MouseEvent::Init(env, exports);
  VGG::NodeAdapter::KeyboardEvent::Init(env, exports);
  VGG::NodeAdapter::TouchEvent::Init(env, exports);
  return nullptr;
}

static napi_module local_linked_napi = {
  NAPI_MODULE_VERSION,
  node::ModuleFlags::kLinked,
  nullptr,
  InitializeLocalNapiBinding,
  "vgg_sdk_addon",
  nullptr,
  { 0 },
};

void link_vgg_sdk_addon(node::Environment* env)
{
  AddLinkedBinding(env, local_linked_napi);
}
