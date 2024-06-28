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
#include <cstddef>
#include <exception>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "Application/RunLoop.hpp"
#include "Application/UIOptions.hpp"
#include "Application/VggSdk.hpp"
#include "ISdk.hpp"
#include "PlatformAdapter/Native/Sdk/AdapterHelper.hpp"
#include <rxcpp/rx-includes.hpp>
#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-observer.hpp>
#include <rxcpp/rx-operators.hpp>
#include <rxcpp/rx-predef.hpp>
#include <rxcpp/rx-sources.hpp>
#include <rxcpp/rx-subscriber.hpp>
#include <rxcpp/sources/rx-iterate.hpp>

namespace VGG
{

constexpr auto listener_code_key = "listener";

using namespace adapter;

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

namespace
{

template<class ResultType, class OnError = rxcpp::detail::OnErrorEmpty>
class SyncTaskInMainLoop
{
public:
  using TaskFunction = std::function<ResultType()>;
  using OnResult = std::function<void(ResultType)>;

  SyncTaskInMainLoop(TaskFunction taskFunction, OnResult onResult, OnError onError = OnError())
    : m_task_function{ std::move(taskFunction) }
    , m_on_result{ std::move(onResult) }
    , m_on_error{ std::move(onError) }
  {
  }

  void operator()()
  {
    auto observable = rxcpp::make_observable_dynamic<ResultType>(
      [&task_funcion = m_task_function](rxcpp::subscriber<ResultType> o)
      {
        try
        {
          auto result = task_funcion();
          o.on_next(std::move(result));
          o.on_completed();
        }
        catch (...)
        {
          o.on_error(std::current_exception());
        }
      });

    observable.subscribe_on(RunLoop::sharedInstance()->thread())
      .as_blocking()
      .subscribe(m_on_result, m_on_error);
  }

private:
  TaskFunction m_task_function;
  OnResult     m_on_result;
  OnError      m_on_error;
};

const ISdk::AnimationOptions animationOptionsFromJsObject(napi_env env, napi_value value)
{
  ISdk::AnimationOptions options;
  {
    bool hasProperty = false;
    napi_has_named_property(env, value, "duration", &hasProperty);
    if (hasProperty)
    {
      napi_value durationValue;
      napi_get_named_property(env, value, "duration", &durationValue);
      napi_get_value_double(env, durationValue, &options.duration);
    }
  }
  {
    bool hasProperty = false;
    napi_has_named_property(env, value, "type", &hasProperty);
    if (hasProperty)
    {
      napi_value typeValue;
      napi_get_named_property(env, value, "type", &typeValue);
      options.type = GetArgString(env, typeValue);
    }
  }
  {
    bool hasProperty = false;
    napi_has_named_property(env, value, "timingFunction", &hasProperty);
    if (hasProperty)
    {
      napi_value timingFunctionValue;
      napi_get_named_property(env, value, "timingFunction", &timingFunctionValue);
      options.timingFunction = GetArgString(env, timingFunctionValue);
    }
  }

  return options;
}

const ISdk::StateOptions toStateOptions(napi_env env, napi_value value)
{
  ISdk::StateOptions options;
  {
    bool hasProperty = false;
    napi_has_named_property(env, value, "resetScrollPosition", &hasProperty);
    if (hasProperty)
    {
      napi_value v;
      napi_get_named_property(env, value, "resetScrollPosition", &v);
      napi_get_value_bool(env, v, &options.resetScrollPosition);
    }
  }
  {
    bool hasProperty = false;
    napi_has_named_property(env, value, "animation", &hasProperty);
    if (hasProperty)
    {
      napi_value v;
      napi_get_named_property(env, value, "animation", &v);
      options.animation = animationOptionsFromJsObject(env, v);
    }
  }

  return options;
}

const ISdk::FrameOptions toFrameOptions(napi_env env, napi_value value)
{
  return toStateOptions(env, value);
}

const app::PopOptions toPopOptions(napi_env env, napi_value value)
{
  app::PopOptions options;
  {
    bool hasProperty = false;
    napi_has_named_property(env, value, "resetScrollPosition", &hasProperty);
    if (hasProperty)
    {
      napi_value v;
      napi_get_named_property(env, value, "resetScrollPosition", &v);
      napi_get_value_bool(env, v, &options.resetScrollPosition);
    }
  }
  {
    bool hasProperty = false;
    napi_has_named_property(env, value, "resetState", &hasProperty);
    if (hasProperty)
    {
      napi_value v;
      napi_get_named_property(env, value, "resetState", &v);
      napi_get_value_bool(env, v, &options.resetState);
    }
  }

  return options;
}

} // namespace

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
  VggSdkNodeAdapter* sdkAdapter = static_cast<VggSdkNodeAdapter*>(nativeObject);
  delete sdkAdapter;
}

void VggSdkNodeAdapter::Init(napi_env env, napi_value exports)
{
  napi_property_descriptor properties[] = {
    DECLARE_NODE_API_PROPERTY("setEnv", SetEnv),
    DECLARE_NODE_API_PROPERTY("setFitToViewportEnabled", setFitToViewportEnabled),

    DECLARE_NODE_API_PROPERTY("launchFrameId", launchFrameId),
    DECLARE_NODE_API_PROPERTY("setLaunchFrameById", setLaunchFrameId),
    DECLARE_NODE_API_PROPERTY("currentTheme", currentTheme),
    DECLARE_NODE_API_PROPERTY("setCurrentTheme", setCurrentTheme),

    DECLARE_NODE_API_PROPERTY("getElement", GetElement),
    DECLARE_NODE_API_PROPERTY("updateElement", UpdateElement),
    DECLARE_NODE_API_PROPERTY("updateElementProperties", updateElementProperties),
    DECLARE_NODE_API_PROPERTY("getDesignDocument", GetDesignDocument),

    DECLARE_NODE_API_PROPERTY("pushFrame", pushFrame),
    DECLARE_NODE_API_PROPERTY("popFrame", popFrame),
    DECLARE_NODE_API_PROPERTY("presentFrame", presentFrame),
    DECLARE_NODE_API_PROPERTY("dismissFrame", dismissFrame),
    DECLARE_NODE_API_PROPERTY("nextFrame", nextFrame),
    DECLARE_NODE_API_PROPERTY("previousFrame", previousFrame),
    // deprecated
    DECLARE_NODE_API_PROPERTY("setCurrentFrameById", setCurrentFrameById),
    DECLARE_NODE_API_PROPERTY("setCurrentFrameByIdAnimated", setCurrentFrameByIdAnimated),
    DECLARE_NODE_API_PROPERTY("presentFrameById", presentFrameById),
    DECLARE_NODE_API_PROPERTY("goBack", goBack),

    DECLARE_NODE_API_PROPERTY("setState", setState),
    DECLARE_NODE_API_PROPERTY("presentState", presentState),
    DECLARE_NODE_API_PROPERTY("dismissState", dismissState),

    DECLARE_NODE_API_PROPERTY("valueAt", DesignDocumentValueAt),

    DECLARE_NODE_API_PROPERTY("addEventListener", AddEventListener),
    DECLARE_NODE_API_PROPERTY("removeEventListener", RemoveEventListener),
    DECLARE_NODE_API_PROPERTY("getEventListeners", GetEventListeners),

    DECLARE_NODE_API_PROPERTY("openUrl", openUrl),

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
    VggSdkNodeAdapter* sdkAdapter = new VggSdkNodeAdapter();
    sdkAdapter->m_env = env;

    NODE_API_CALL(
      env,
      napi_wrap(
        env,
        _this,
        sdkAdapter,
        VggSdkNodeAdapter::Destructor,
        nullptr /* finalize_hint */,
        &sdkAdapter->m_wrapper));

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

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, envKey]()
                              {
                                sdk->setEnv(envKey);
                                return true;
                              },
                              [](bool) {} }();
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::setFitToViewportEnabled(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 1;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto enabled = GetArgBoolValue(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, enabled]()
                              {
                                sdk->setFitToViewportEnabled(enabled);
                                return true;
                              },
                              [&success](bool result) { success = result; } }();

    return nullptr;
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

    std::string value;
    SyncTaskInMainLoop<bool>{ [&value, sdk = sdkAdapter->m_vggSdk, id]()
                              {
                                value = sdk->getElement(id);
                                return true;
                              },
                              [](bool) {} }();

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

    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, id, contentPatch]()
                              {
                                sdk->updateElement(id, contentPatch);
                                return true;
                              },
                              [](bool) {} }();
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::setCurrentFrameById(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 2;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto id = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, id]()
                              { return sdk->setCurrentFrameById(id, true); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::setCurrentFrameByIdAnimated(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 3;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto id = GetArgString(env, args[0]);
    const auto options = animationOptionsFromJsObject(env, args[2]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, id, &options]()
                              { return sdk->setCurrentFrameByIdAnimated(id, true, options); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::presentFrameById(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 2;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto id = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, id]()
                              { return sdk->presentFrameById(id, true); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::dismissFrame(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, NULL));

  try
  {
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk]() { return sdk->dismissFrame(); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::goBack(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, NULL));

  try
  {
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk]() { return sdk->goBack(true, true); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::nextFrame(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, NULL));

  try
  {
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk]() { return sdk->nextFrame(); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::previousFrame(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, NULL));

  try
  {
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk]() { return sdk->previousFrame(); },
                              [&success](bool result) { success = result; } }();

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

// instance state
napi_value VggSdkNodeAdapter::setState(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 4;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto& instanceDescendantId = GetArgString(env, args[0]);
    const auto& listenerId = GetArgString(env, args[1]);
    const auto& newMasterId = GetArgString(env, args[2]);
    const auto& opts = toStateOptions(env, args[3]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{
      [&sdk = sdkAdapter->m_vggSdk, &instanceDescendantId, &listenerId, &newMasterId, &opts]()
      { return sdk->setState(instanceDescendantId, listenerId, newMasterId, opts); },
      [&success](bool result) { success = result; }
    }();

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

napi_value VggSdkNodeAdapter::presentState(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 4;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto& instanceDescendantId = GetArgString(env, args[0]);
    const auto& listenerId = GetArgString(env, args[1]);
    const auto& newMasterId = GetArgString(env, args[2]);
    const auto& opts = toStateOptions(env, args[3]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{
      [&sdk = sdkAdapter->m_vggSdk, &instanceDescendantId, &listenerId, &newMasterId, &opts]()
      { return sdk->presentState(instanceDescendantId, listenerId, newMasterId, opts); },
      [&success](bool result) { success = result; }
    }();

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

napi_value VggSdkNodeAdapter::dismissState(napi_env env, napi_callback_info info)
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
    auto instanceDescendantId = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, instanceDescendantId]()
                              { return sdk->dismissState(instanceDescendantId); },
                              [&success](bool result) { success = result; } }();

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

  VggSdkNodeAdapter* sdkAdapter;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

  std::string json;
  SyncTaskInMainLoop<bool>{ [&json, sdk = sdkAdapter->m_vggSdk]()
                            {
                              json = sdk->designDocument();
                              return true;
                            },
                            [](bool) {} }();

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
    auto jsonPointer = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    std::string value;
    SyncTaskInMainLoop<bool>{ [&value, sdk = sdkAdapter->m_vggSdk, jsonPointer]()
                              {
                                value = sdk->designDocumentValueAt(jsonPointer);
                                return true;
                              },
                              [](bool) {} }();

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
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    auto elementKey = GetArgString(env, args[0]);
    auto eventType = GetArgString(env, args[1]);
    auto listenerCode = GetArgString(env, args[2]);

    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, elementKey, eventType, listenerCode]()
                              {
                                sdk->addEventListener(elementKey, eventType, listenerCode);
                                return true;
                              },
                              [](bool) {} }();
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
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    auto elementKey = GetArgString(env, args[0]);
    auto eventType = GetArgString(env, args[1]);
    auto listenerCode = GetArgString(env, args[2]);

    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, elementKey, eventType, listenerCode]()
                              {
                                sdk->removeEventListener(elementKey, eventType, listenerCode);
                                return true;
                              },
                              [](bool) {} }();
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

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    napi_value result_listeners_map; // result object
    napi_create_object(env, &result_listeners_map);

    VGG::VggSdk::ListenersType listenersMap;
    SyncTaskInMainLoop<bool>{ [&listenersMap, sdk = sdkAdapter->m_vggSdk, elementKey]()
                              {
                                listenersMap = sdk->getEventListeners(elementKey);
                                return true;
                              },
                              [](bool) {} }();

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
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk]()
                              {
                                sdk->save();
                                return true;
                              },
                              [](bool) {} }();

    return nullptr;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
    return nullptr;
  }
}

napi_value VggSdkNodeAdapter::launchFrameId(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, NULL));

  try
  {
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    std::string result;
    SyncTaskInMainLoop<bool>{ [&sdk = sdkAdapter->m_vggSdk, &result]()
                              {
                                result = sdk->launchFrameId();
                                return true;
                              },
                              [](bool) {} }();

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, result.data(), result.size(), &ret));
    return ret;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}
napi_value VggSdkNodeAdapter::setLaunchFrameId(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 1;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto id = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [&sdk = sdkAdapter->m_vggSdk, &id]()
                              { return sdk->setLaunchFrameById(id); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::openUrl(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 2;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto url = GetArgString(env, args[0]);
    const auto target = GetArgString(env, args[1]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    SyncTaskInMainLoop<bool>{ [&sdk = sdkAdapter->m_vggSdk, &url, &target]()
                              {
                                sdk->openUrl(url, target);
                                return true;
                              },
                              [](bool) {} }();
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

napi_value VggSdkNodeAdapter::currentTheme(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, NULL));

  try
  {
    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    std::string result;
    SyncTaskInMainLoop<bool>{ [&sdk = sdkAdapter->m_vggSdk, &result]()
                              {
                                result = sdk->currentTheme();
                                return true;
                              },
                              [](bool) {} }();

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, result.data(), result.size(), &ret));
    return ret;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}
napi_value VggSdkNodeAdapter::setCurrentTheme(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 1;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto themeName = GetArgString(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [&sdk = sdkAdapter->m_vggSdk, &themeName]()
                              { return sdk->setCurrentTheme(themeName); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::pushFrame(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 2;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto& id = GetArgString(env, args[0]);
    const auto& options = toFrameOptions(env, args[1]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, &id, &options]()
                              { return sdk->pushFrame(id, options); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::popFrame(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 1;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto& options = toPopOptions(env, args[0]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, &options]()
                              { return sdk->popFrame(options); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::presentFrame(napi_env env, napi_callback_info info)
{
  constexpr size_t ARG_COUNT = 2;
  size_t           argc = ARG_COUNT;
  napi_value       args[ARG_COUNT];
  napi_value       _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc < ARG_COUNT)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    const auto& id = GetArgString(env, args[0]);
    const auto& options = toFrameOptions(env, args[1]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    bool success{ false };
    SyncTaskInMainLoop<bool>{ [sdk = sdkAdapter->m_vggSdk, &id, &options]()
                              { return sdk->presentFrame(id, options); },
                              [&success](bool result) { success = result; } }();

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

napi_value VggSdkNodeAdapter::updateElementProperties(napi_env env, napi_callback_info info)
{
  size_t     argc = 2;
  napi_value args[2];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  if (argc != 2)
  {
    napi_throw_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  try
  {
    auto       updates = GetArgString(env, args[0]);
    const auto option = animationOptionsFromJsObject(env, args[1]);

    VggSdkNodeAdapter* sdkAdapter;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&sdkAdapter)));

    int successCount = 0;
    SyncTaskInMainLoop<bool>{ [&successCount, sdk = sdkAdapter->m_vggSdk, updates, option]()
                              {
                                successCount = sdk->updateElementProperties(updates, option);
                                return true;
                              },
                              [](bool) {} }();

    napi_value value;
    auto       status = napi_create_int32(env, successCount, &value);
    if (status == napi_ok)
      return value;
  }
  catch (std::exception& e)
  {
    napi_throw_error(env, nullptr, e.what());
  }

  return nullptr;
}

} // namespace VGG