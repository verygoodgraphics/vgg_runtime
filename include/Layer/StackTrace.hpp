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

#include <stack>
#include <string>
#include <iostream>
#include "Layer/Config.hpp"
namespace VGG::layer
{

#define VGG_TRACE_LOG(...) VGG_LOG(TRACE, StackTrace, __VA_ARGS__)

class StackTrace
{
public:
  template<typename... Args>
  StackTrace(const char* frameInfo, const char* funcName, Args&&... args)
  {
#if __GUNC__ >= 13
    std::string info =
      std::format("{} in {}: {}", frameInfo, funcName, std::forward<Args>(args)...);
#else
    std::string info = frameInfo;
    info += funcName;
#endif
    s_functionStack.push(info);
    VGG_TRACE_LOG("{}", info);
  }

  ~StackTrace()
  {
    s_functionStack.pop();
  }

  static int printStackTrace()
  {
    std::stack<std::string> tempStack = s_functionStack;
    auto                    depth = tempStack.size();
    printStackTraceReverse(tempStack);
    return depth;
  }

private:
  inline static thread_local std::stack<std::string> s_functionStack = {};

  static void printStackTraceReverse(std::stack<std::string>& stack)
  {
    if (stack.empty())
      return;
    auto top = stack.top();
    stack.pop();
    printStackTraceReverse(stack);
    std::string indent(stack.size(), ' ');
    VGG_TRACE_LOG("{}{}", indent, top);
  }
};
#undef VGG_TRACE_LOG

} // namespace VGG::layer

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define VGG_TRACE_IMPL(msg) VGG::layer::StackTrace __TRACE_VAR(AT, __FUNCTION__, msg);

#ifdef VGG_NDEBUG
#define VGG_TRACE_DEV
#else
#define VGG_TRACE_DEV(x) VGG_TRACE_IMPL(x)
#endif

#define VGG_TRACE(x) VGG_TRACE_IMPL(x)
