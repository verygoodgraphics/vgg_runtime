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
#include "AdapterHelper.hpp"
#include "defines.hpp"
#include <cassert>
#include <stdexcept>

namespace VGG
{

namespace adapter
{

std::string GetArgString(napi_env env, napi_value arg)
{
  napi_valuetype value_type;
  NODE_API_CALL(env, napi_typeof(env, arg, &value_type));
  if (value_type != napi_string)
  {
    throw std::invalid_argument("Wrong argument type. Strings expected.");
  }

  size_t len = 0;
  auto status = napi_get_value_string_utf8(env, arg, NULL, 0, &len);
  assert(status == napi_ok);

  char* buf = (char*)malloc(len + 1);
  status = napi_get_value_string_utf8(env, arg, buf, len + 1, &len);
  assert(status == napi_ok);

  std::string result;
  result.append(buf);

  free(buf);

  return result;
}

int GetArgIntValue(napi_env env, napi_value arg)
{
  napi_valuetype value_type;
  NODE_API_CALL(env, napi_typeof(env, arg, &value_type));
  if (value_type != napi_number)
  {
    throw std::invalid_argument("Wrong argument type. Number expected.");
  }

  int32_t result = 0;
  auto status = napi_get_value_int32(env, arg, &result);
  assert(status == napi_ok);

  return result;
}

} // namespace adapter

} // namespace VGG
