#include "node.h"
#include "node_api.h"
#include "util.h"

#include "PlatformAdapter/Native/Sdk/VggSdkAddon.hpp"
#include "PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.hpp"

napi_value InitializeLocalNapiBinding(napi_env env, napi_value exports)
{
  VggSdkNodeAdapter::Init(env, exports);
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
