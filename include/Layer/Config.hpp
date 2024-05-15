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

namespace VGG::layer
{
FILE* getLogStream(const char* category);
}

#define STRINGIFY(x) #x

#if __GNUC__ >= 13
#include <iostream>
#include <format>

#define VGG_TRACE_INFO_IMPL(category, label, ...)                                                  \
  do                                                                                               \
  {                                                                                                \
    auto f = layer::getLogStream(category);                                                        \
    auto s = std::format(__VA_ARGS__);                                                             \
    if (!s.empty())                                                                                \
    {                                                                                              \
      fprintf(f, "[" STRINGIFY(label) "]%s\n", s.c_str());                                         \
    }                                                                                              \
  } while (0);
#else
#define VGG_TRACE_INFO_IMPL(category, label, ...)
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
#define VGG_TRACE_INFO(...)
#define VGG_LAYER_LOG(...)
#else
#define VGG_LAYER_DEBUG
#define VGG_LAYER_DEBUG_CODE(...)                                                                  \
  do                                                                                               \
  {                                                                                                \
    __VA_ARGS__;                                                                                   \
  } while (0);

#define VGG_TRACE_INFO(category, label, ...) VGG_TRACE_INFO_IMPL(category, label, __VA_ARGS__)
#define VGG_LAYER_LOG(...) VGG_TRACE_INFO_IMPL("log", LAYER_INFO, __VA_ARGS__)
#endif
