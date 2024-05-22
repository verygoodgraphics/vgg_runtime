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
#pragma once

#include <cstdio>

#include <string_view>
#include <vector>
#include <mutex>

namespace VGG::layer
{
FILE* getLogStream(const char* category);

} // namespace VGG::layer

#define STRINGIFY(x) #x
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define UNI_NAME(prefix) CONCAT(prefix, __LINE__)

#if __GNUC__ >= 13
#include <iostream>
#include <format>
#define VGG_LOG_IMPL(line, category, label, ...)                                                   \
  do                                                                                               \
  {                                                                                                \
    auto UNI_NAME(f) = layer::getLogStream(#category);                                             \
    if (UNI_NAME(f) == nullptr)                                                                    \
      break;                                                                                       \
    auto UNI_NAME(s) = std::format(__VA_ARGS__);                                                   \
    if (!UNI_NAME(s).empty())                                                                      \
    {                                                                                              \
      fprintf(UNI_NAME(f), "[" STRINGIFY(label) "]%s\n", UNI_NAME(s).c_str());                     \
    }                                                                                              \
  } while (0);
#else
#define VGG_LOG_IMPL(line, category, label, ...) (void)sizeof(__VA_ARGS__);
#endif

#if defined(_WIN32) && defined(LAYER_SHARED_LIBRARY)
#ifdef layer_EXPORTS
#define VGG_EXPORTS __declspec(dllexport)
#else
#define VGG_EXPORTS __declspec(dllimport)
#endif
#else
#define VGG_EXPORTS
#endif

#ifdef VGG_NDEBUG
#define VGG_LAYER_DEBUG_CODE(code)
#define VGG_LAYER_LOG(...)
#define VGG_LOG_DEV(...)
#else
#define VGG_LAYER_DEBUG
#define VGG_LAYER_DEBUG_CODE(...)                                                                  \
  do                                                                                               \
  {                                                                                                \
    __VA_ARGS__;                                                                                   \
  } while (0);

#define VGG_LOG_DEV(category, label, ...) VGG_LOG_IMPL(__LINE__, category, label, __VA_ARGS__)
#endif

#define VGG_LOG(category, label, ...) VGG_LOG_IMPL(__LINE__, category, label, __VA_ARGS__)
