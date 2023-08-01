#pragma once

#include <string>
#include <vector>

#include "node_api.h"

namespace VGG
{

namespace adapter
{

std::string GetArgString(napi_env env, napi_value arg);
int GetArgIntValue(napi_env env, napi_value arg);

} // namespace adapter

} // namespace VGG