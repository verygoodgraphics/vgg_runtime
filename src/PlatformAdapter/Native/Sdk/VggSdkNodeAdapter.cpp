#include "VggSdkNodeAdapter.hpp"

#include "VggSdk.hpp"
#include "VggDepContainer.hpp"

#include <string>

// Empty value so that macros here are able to return NULL or void
#define NODE_API_RETVAL_NOTHING // Intentionally blank #define

#define GET_AND_THROW_LAST_ERROR(env)                                                              \
  do                                                                                               \
  {                                                                                                \
    const napi_extended_error_info* error_info;                                                    \
    napi_get_last_error_info((env), &error_info);                                                  \
    bool is_pending;                                                                               \
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
  : m_vggSdk(VggDepContainer<std::shared_ptr<VggSdk>>::get())
  , m_env(nullptr)
  , m_wrapper(nullptr)
{
}

VggSdkNodeAdapter::~VggSdkNodeAdapter()
{
  napi_delete_reference(m_env, m_wrapper);
}

void VggSdkNodeAdapter::Destructor(napi_env env, void* nativeObject, void* /*finalize_hint*/)
{
  VggSdkNodeAdapter* obj = static_cast<VggSdkNodeAdapter*>(nativeObject);
  delete obj;
}

void VggSdkNodeAdapter::Init(napi_env env, napi_value exports)
{
  napi_property_descriptor properties[] = {
    DECLARE_NODE_API_PROPERTY("updateStyle", UpdateStyle),

    DECLARE_NODE_API_PROPERTY("getDocumentJson", GetDocumentJson),
    DECLARE_NODE_API_PROPERTY("jsonAt", GetJsonAt),

    DECLARE_NODE_API_PROPERTY("getElementPath", GetElementPath),
    DECLARE_NODE_API_PROPERTY("getElementContainerPath", GetElementContainerPath),
    DECLARE_NODE_API_PROPERTY("findElement", FindElement),

    DECLARE_NODE_API_PROPERTY("replaceInDocument", ReplaceInDocument),
    DECLARE_NODE_API_PROPERTY("addToDocument", AddToDocument),
    DECLARE_NODE_API_PROPERTY("deleteFromDocument", DeleteFromDocument),
  };

  napi_value cons;
  NODE_API_CALL_RETURN_VOID(env,
                            napi_define_class(env,
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

  size_t argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  if (is_constructor)
  {
    // Invoked as constructor: `new VggSdk()`
    VggSdkNodeAdapter* obj = new VggSdkNodeAdapter();
    obj->m_env = env;

    NODE_API_CALL(env,
                  napi_wrap(env,
                            _this,
                            obj,
                            VggSdkNodeAdapter::Destructor,
                            nullptr /* finalize_hint */,
                            &obj->m_wrapper));

    return _this;
  }

  // Invoked as plain function `VggSdk()`, turn into construct call.
  napi_value cons;
  NODE_API_CALL(env, napi_get_reference_value(env, constructor, &cons));

  napi_value instance;
  NODE_API_CALL(env, napi_new_instance(env, cons, 0, nullptr, &instance));

  return instance;
}

napi_value VggSdkNodeAdapter::UpdateStyle(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  obj->m_vggSdk->updateStyle();

  return nullptr;
}

napi_value VggSdkNodeAdapter::GetDocumentJson(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  auto& json = obj->m_vggSdk->documentJson();

  napi_value ret;
  NODE_API_CALL(env, napi_create_string_utf8(env, json.data(), json.size(), &ret));

  return ret;
}

napi_value VggSdkNodeAdapter::GetJsonAt(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  NODE_API_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NODE_API_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. Strings expected.");

  std::string json_pointer_string;
  GetArgString_(env, json_pointer_string, args[0]);

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  // auto& json = obj->m_vggSdk->jsonAt(json_pointer_string);

  napi_value ret;
  // NODE_API_CALL(env, napi_create_string_utf8(env, json.data(), json.size(), &ret));

  return ret;
}

napi_value VggSdkNodeAdapter::GetElementPath(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  NODE_API_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NODE_API_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. Strings expected.");

  std::string arg1_string;
  GetArgString_(env, arg1_string, args[0]);

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  // auto& sdk_ret = obj->m_vggSdk->getElementPath(arg1_string);
  napi_value ret;
  // NODE_API_CALL(env, napi_create_string_utf8(env, sdk_ret.data(), sdk_ret.size(), &ret));

  return ret;
}

napi_value VggSdkNodeAdapter::GetElementContainerPath(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  NODE_API_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NODE_API_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. Strings expected.");

  std::string arg1_string;
  GetArgString_(env, arg1_string, args[0]);

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  // auto& sdk_ret = obj->m_vggSdk->getElementContainerPath(arg1_string);
  napi_value ret;
  // NODE_API_CALL(env, napi_create_string_utf8(env, sdk_ret.data(), sdk_ret.size(), &ret));

  return ret;
}

napi_value VggSdkNodeAdapter::FindElement(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  NODE_API_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NODE_API_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. Strings expected.");

  std::string arg1_string;
  GetArgString_(env, arg1_string, args[0]);

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  // auto& sdk_ret = obj->m_vggSdk->findElement(arg1_string);
  napi_value ret;
  // NODE_API_CALL(env, napi_create_string_utf8(env, sdk_ret.data(), sdk_ret.size(), &ret));

  return ret;
}

napi_value VggSdkNodeAdapter::ReplaceInDocument(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[2];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  NODE_API_ASSERT(env, argc >= 2, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));

  napi_valuetype valuetype1;
  NODE_API_CALL(env, napi_typeof(env, args[1], &valuetype1));

  NODE_API_ASSERT(env,
                  valuetype0 == napi_string && valuetype1 == napi_string,
                  "Wrong argument type. Strings expected.");

  std::string json_pointer_string;
  GetArgString_(env, json_pointer_string, args[0]);

  std::string json_value_string;
  GetArgString_(env, json_value_string, args[1]);

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));
  // obj->m_vggSdk->replaceInDocument(json_pointer_string, json_value_string);

  return nullptr;
}

napi_value VggSdkNodeAdapter::AddToDocument(napi_env env, napi_callback_info info)
{
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  return nullptr;
}

napi_value VggSdkNodeAdapter::DeleteFromDocument(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  napi_value _this;
  NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, NULL));

  NODE_API_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NODE_API_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. Strings expected.");

  std::string json_pointer_string;
  GetArgString_(env, json_pointer_string, args[0]);

  VggSdkNodeAdapter* obj;
  NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));
  obj->m_vggSdk->deleteFromDocument(json_pointer_string);

  return nullptr;
}

void VggSdkNodeAdapter::GetArgString_(napi_env env, std::string& to_string, napi_value arg)
{
  size_t len = 0;
  auto status = napi_get_value_string_utf8(env, arg, NULL, 0, &len);
  assert(status == napi_ok);

  char* buf = (char*)malloc(len + 1);
  status = napi_get_value_string_utf8(env, arg, buf, len + 1, &len);
  assert(status == napi_ok);
  to_string.append(buf);

  free(buf);
}
