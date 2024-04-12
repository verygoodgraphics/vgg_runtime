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
#ifndef VGG_SDK_NODE_ADAPTER_HPP
#define VGG_SDK_NODE_ADAPTER_HPP

#include <memory>
#include <vector>

#include <node_api.h>

namespace VGG
{

class VggSdk;

class VggSdkNodeAdapter
{
public:
  static void Init(napi_env env, napi_value exports);
  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

private:
  VggSdkNodeAdapter();
  ~VggSdkNodeAdapter();

  static napi_value New(napi_env env, napi_callback_info info);

  // env
  static napi_value SetEnv(napi_env env, napi_callback_info info);

  // design document in vgg daruma file
  static napi_value GetElement(napi_env env, napi_callback_info info);
  static napi_value UpdateElement(napi_env env, napi_callback_info info);

  static napi_value setCurrentFrameById(napi_env env, napi_callback_info info);
  static napi_value presentFrameById(napi_env env, napi_callback_info info);
  static napi_value dismissFrame(napi_env env, napi_callback_info info);

  // instance state
  static napi_value setMasterId(napi_env env, napi_callback_info info);
  static napi_value presentState(napi_env env, napi_callback_info info);
  static napi_value dismissState(napi_env env, napi_callback_info info);

  //
  static napi_value GetDesignDocument(napi_env env, napi_callback_info info);

  static napi_value DesignDocumentValueAt(napi_env env, napi_callback_info info);

  // event listener
  static napi_value AddEventListener(napi_env env, napi_callback_info info);
  static napi_value RemoveEventListener(napi_env env, napi_callback_info info);
  static napi_value GetEventListeners(napi_env env, napi_callback_info info);

  // undo & redo
  static napi_value Undo(napi_env env, napi_callback_info info);
  static napi_value Redo(napi_env env, napi_callback_info info);
  static napi_value Save(napi_env env, napi_callback_info info);

  static napi_ref constructor;

  napi_env                m_env;
  napi_ref                m_wrapper;
  std::shared_ptr<VggSdk> m_vggSdk;
};

} // namespace VGG

#endif
