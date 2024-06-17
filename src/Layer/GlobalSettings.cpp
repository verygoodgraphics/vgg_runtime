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

#include "Layer/GlobalSettings.hpp"
#include "Layer/Config.hpp"

#include <stdlib.h>
#include <initializer_list>

namespace
{
bool g_enableAnimatedPattern = false;
}

namespace VGG::layer
{

void setAnimatedPatternEnabled(bool enable)
{
  g_enableAnimatedPattern = enable;
}

bool isAnimatedPatternEnabled()
{
  return g_enableAnimatedPattern;
}

void setupEnv()
{
  static struct
  {
    const char* name;
    const char* value;
  } s_var = { "SKPARAGRAPH_REMOVE_ROUNDING_HACK", "1" };
#ifdef _WIN32
  for (const auto& var : { s_var })
  {
    if (_putenv_s(var.name, var.value) != 0)
    {
      VGG_LOG_DEV(LOG, Setup, "Failed to set environment variable {}={}", var.name, var.value);
    }
  }
#else
  for (const auto& var : { s_var })
  {
    if (setenv(var.name, var.value, 1) != 0)
    {
      VGG_LOG_DEV(LOG, Setup, "Failed to set environment variable {}={}", var.name, var.value);
    }
  }
#endif
}

} // namespace VGG::layer
