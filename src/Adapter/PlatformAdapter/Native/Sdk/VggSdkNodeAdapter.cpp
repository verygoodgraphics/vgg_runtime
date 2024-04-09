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
#include "PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.hpp"

#include "PlatformAdapter/Native/Sdk/AdapterHelper.hpp"

#include "Application/VggSdk.hpp"

#include <string>
#include <cassert>

constexpr auto listener_code_key = "listener";

using namespace VGG::adapter;
using namespace VGG;

// Empty value so that macros here are able to return NULL or void
#define NODE_API_RETVAL_NOTHING // Intentionally blank #define

#define GET_AND_THROW_LAST_ERROR(env)                                                              \
  do                                                                                               \
  {                                                                                                \
    const napi_extended_error_info* error_info;                                                    \
    napi_get_last_error_info((env), &error_info);                                                  \
    bool        is_pending;                                                                        \
    const char* err_message = error_info->error_message;                                           \
    napi_is_exception_pending((env), &is_pending);                                                 \
    /* If an exception is already pending, don't rethrow it */                                     \
    if (!is_pending)                                                                               \
    {                                                                                              \
      const char* error_message = err_message != NULL ? err_message : "empty error message";       \
      napi_throw_error((env), NULL, error_message);                                                \
    }                                                                                              \
  } while (0)

#define NODE_API_ASSERT_BASE(env, assertion, message, ret_val)                                     \
  do                                                                                               \
  {                                                                                                \
    if (!(assertion))                                                                              \
    {                                                                                              \
      napi_throw_error((env), NULL, "assertion (" #assertion ") failed: " message);                \
      return ret_val;                                                                              \
    }                                                                                              \
  } while (0)

// Returns NULL on failed assertion.
// This is meant to be used inside napi_callback methods.
#define NODE_API_ASSERT(env, assertion, message) NODE_API_ASSERT_BASE(env, assertion, message, NULL)

// Returns empty on failed assertion.
// This is meant to be used inside functions with void return type.
#define NODE_API_ASSERT_RETURN_VOID(env, assertion, message)                                       \
  NODE_API_ASSERT_BASE(env, assertion, message, NODE_API_RETVAL_NOTHING)

#define NODE_API_CALL_BASE(env, the_call, ret_val)                                                 \
  do                                                                                               \
  {                                                                                                \
    if ((the_call) != napi_ok)                                                                     \
    {                                                                                              \
      GET_AND_THROW_LAST_ERROR((env));                                                             \
      return ret_val;                                                                              \
    }                                                                                              \
  } while (0)

// Returns NULL if the_call doesn't return napi_ok.
#define NODE_API_CALL(env, the_call) NODE_API_CALL_BASE(env, the_call, NULL)

// Returns empty if the_call doesn't return napi_ok.
#define NODE_API_CALL_RETURN_VOID(env, the_call)                                                   \
  NODE_API_CALL_BASE(env, the_call, NODE_API_RETVAL_NOTHING)

#define DECLARE_NODE_API_PROPERTY(name, func)                                                      \
  {                                                                                                \
    (name), NULL, (func), NULL, NULL, NULL, napi_default, NULL                                     \
  }

#define DECLARE_NODE_API_GETTER(name, func)                                                        \
  {                                                                                                \
    (name), NULL, NULL, (func), NULL, NULL, napi_default, NULL                                     \
  }

#define DECLARE_NODE_API_PROPERTY_VALUE(name, value)                                               \
  {                                                                                                \
    (name), NULL, NULL, NULL, NULL, (value), napi_default, NULL                                    \
  }

napi_ref VggSdkNodeAdapter::constructor;

VggSdkNodeAdapter::VggSdkNodeAdapter()
  : m_env(nullptr)
  , m_wrapper(nullptr)
  , m_vggSdk(new VggSdk) // todo, env
{
}

VggSdkNodeAdapter::~VggSdkNodeAdapter()
{
  napi_delete_reference(m_env, m_wrapper);
}

void VggSdkNodeAdapter::Destructor(napi_env env, void* nativeObject, void* /*finalize_hint*/)
{
  VggSdkNodeAdapter* sdk_adapter = static_cast<VggSdkNodeAdapter*>(nativeObject);
  delete sdk_adapter;
}

void VggSdkNodeAdapter::Init(napi_env env, napi_value exports)
{
  napi_property_descriptor properties[] = {
    DECLARE_NODE_API_PROPERTY("setEnv", SetEnv),

    DECLARE_NODE_API_PROPERTY("getElement", GetElement),
    DECLARE_NODE_API_PROPERTY("updateElement", UpdateElement),
    DECLARE_NODE_API_PROPERTY("getDesignDocument", GetDesignDocument),

    DECLARE_NODE_API_PROPERTY("setCurrentFrame", setCurrentFrame),
    DECLARE_NODE_API_PROPERTY("setCurrentFrameById", setCurrentFrameById),

    DECLARE_NODE_API_PROPERTY("valueAt", DesignDocumentValueAt),

    DECLARE_NODE_API_PROPERTY("addEventListener", AddEventListener),
    DECLARE_NODE_API_PROPERTY("removeEventListener", RemoveEventListener),
    DECLARE_NODE_API_PROPERTY("getEventListeners", GetEventListeners),

    DECLARE_NODE_API_PROPERTY("save", Save),
  };

  napi_value cons;
  NODE_API_CALL_RETURN_VOID(
    env,
    napi_define_class(
      env,
      "VggSdk",
      -1,
      New,
      nullptr,
      sizeof(properties) / sizeof(napi_property_descriptor),
      properties,
      &cons));

  NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &constructor));

  NODE_API_CALL_RETURN_VOID(env, napi_set_named_property(env, exports, "VggSdk", cons));
}

napi_value VggSdkNodeAdapter::New(napi_env env, napi_callback_info info)
{
  napi_value new_target;
  NODE_API_CALL(env, napi_get_new_target(env, info, &new_target));
  bool is_constructor = (new_target != nullptr);

  size_t     argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  if (is_constructor)
  {
    // Invoked as constructor: `new VggSdk()`
    VggSdkNodeAdapter* sdk_adapter = new VggSdkNodeAdapter();
    sdk_adapter->m_env = env;

    NODE_API_CALL(
      env,
      napi_wrap(
        env,
        _this,
        sdk_adapter,
        VggSdkNodeAdapter::Destructor,
        nullptr /* finalize_hint */,
        &sdk_adapter->m_wrapper));

    return _this;
  }

  // Invoked as plain function `VggSdk()`, turn into construct call.
  napi_value cons;
  NODE_API_CALL(env, napi_get_reference_value(env, constructor, &cons));

  napi_value instance;
  NODE_API_CALL(env, napi_new_instance(env, cons, 0, nullptr, &instance));

  return instance;
}

napi_value VggSdkNodeAdapter::SetEnv(napi_env env, napi_callback_info info)
{
  size_t     argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 1)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto envKey = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdk_adapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdk_adapter)));

    sdk_adapter->m_vggSdk->setEnv(envKey);
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::GetElement(napi_env env, napi_callback_info info)
{
  size_t     argc = 2;
  napi_value args[2];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 1 && argc != 2)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto id = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    const auto& value = sdkAdapter->m_vggSdk->getElement(id);

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, value.data(), value.size(), &ret));

    return ret;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::UpdateElement(napi_env env, napi_callback_info info)
{
  size_t     argc = 3;
  napi_value args[3];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 2 && argc != 3)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto id = GetArgString(env, args[0]);
    auto contentPatch = GetArgString(env, args[1]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    sdkAdapter->m_vggSdk->updateElement(id, contentPatch);
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::setCurrentFrame(napi_env env, napi_callback_info info)
{
  size_t     argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 1)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto name = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    auto success = sdkAdapter->m_vggSdk->setCurrentFrame(name);

    napi_value ret;
    NODE_API_CALL(env, napi_get_boolean(env, success, &ret));
    return ret;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::setCurrentFrameById(napi_env env, napi_callback_info info)
{
  size_t     argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 1)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto id = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    auto success = sdkAdapter->m_vggSdk->setCurrentFrameById(id);

    napi_value ret;
    NODE_API_CALL(env, napi_get_boolean(env, success, &ret));
    return ret;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::GetDesignDocument(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

  VggSdkNodeAdapter* sdk_adapter;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdk_adapter)));

  const auto& json = sdk_adapter->m_vggSdk->designDocument();

  napi_value ret;
  NODE_API_CALL(env, napi_create_string_utf8(env, json.data(), json.size(), &ret));

  return ret;
}

napi_value VggSdkNodeAdapter::DesignDocumentValueAt(napi_env env, napi_callback_info info)
{
  size_t     argc = 2;
  napi_value args[2];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 1 && argc != 2)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto json_pointer_string = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdk_adapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdk_adapter)));

    const auto& value = sdk_adapter->m_vggSdk->designDocumentValueAt(json_pointer_string);

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, value.data(), value.size(), &ret));

    return ret;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

// event listener
napi_value VggSdkNodeAdapter::AddEventListener(napi_env env, napi_callback_info info)
{
  size_t     argc = 4;
  napi_value args[4];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 3 && argc != 4)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    VggSdkNodeAdapter* sdk_adapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdk_adapter)));

    auto elementKey = GetArgString(env, args[0]);
    auto eventType = GetArgString(env, args[1]);
    auto listenerCode = GetArgString(env, args[2]);

    sdk_adapter->m_vggSdk->addEventListener(elementKey, eventType, listenerCode);
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::RemoveEventListener(napi_env env, napi_callback_info info)
{
  size_t     argc = 4;
  napi_value args[4];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 3 && argc != 4)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    VggSdkNodeAdapter* sdk_adapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdk_adapter)));

    auto elementKey = GetArgString(env, args[0]);
    auto eventType = GetArgString(env, args[1]);
    auto listenerCode = GetArgString(env, args[2]);

    sdk_adapter->m_vggSdk->removeEventListener(elementKey, eventType, listenerCode);
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

/*
google chrome console, getEventListeners result:
 {
  "click": [
    {
      "useCapture": false,
      "passive": false,
      "once": false,
      "type": "click",
      "listener": f
    },
    {
      "useCapture": true,
      "passive": false,
      "once": false,
      "type": "click",
      "listener": f
    }
  ],
  "mousedown": [
    {
      "useCapture": true,
      "passive": false,
      "once": false,
      "type": "mousedown",
      "listener": f
    }
  ],
  "mouseup": [
    {
      "useCapture": true,
      "passive": false,
      "once": false,
      "type": "mouseup",
      "listener": f
    }
  ]
};
*/
napi_value VggSdkNodeAdapter::GetEventListeners(napi_env env, napi_callback_info info)
{
  size_t     argc = 2;
  napi_value args[2];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 1 && argc != 2)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto elementKey = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdk_adapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdk_adapter)));

    napi_value result_listeners_map; // result object
    napi_create_object(env, &result_listeners_map);

    auto listenersMap = sdk_adapter->m_vggSdk->getEventListeners(elementKey);
    for (auto& map_item : listenersMap)
    {
      if (map_item.second.empty())
      {
        continue;
      }

      auto& eventType = map_item.first;

      napi_value js_listener_code_array; // listener array of the eventType
      napi_create_array_with_length(env, map_item.second.size(), &js_listener_code_array);
      for (std::size_t i = 0; i < map_item.second.size(); ++i)
      {
        auto& listenerCode = map_item.second[i];

        napi_value js_listener_object; // array item, listener object
        napi_create_object(env, &js_listener_object);

        napi_value js_listener_code; // code string
        NODE_API_CALL(
          env,
          napi_create_string_utf8(
            env,
            listenerCode.data(),
            listenerCode.size(),
            &js_listener_code));
        NODE_API_CALL(
          env,
          napi_set_named_property(env, js_listener_object, listener_code_key, js_listener_code));

        NODE_API_CALL(env, napi_set_element(env, js_listener_code_array, i, js_listener_object));
      }

      NODE_API_CALL(
        env,
        napi_set_named_property(
          env,
          result_listeners_map,
          eventType.c_str(),
          js_listener_code_array));
    }
    return result_listeners_map;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
    return nullptr;
  }
}

napi_value VggSdkNodeAdapter::Save(napi_env env, napi_callback_info info)
{
  size_t     argc = 0;
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr));

  if (argc != 0)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    VggSdkNodeAdapter* sdk_adapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdk_adapter)));

    sdk_adapter->m_vggSdk->save();

    return nullptr;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
    return nullptr;
  }
}
