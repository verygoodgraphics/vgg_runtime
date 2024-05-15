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

#include "Layer/Config.hpp"
#include "Layer/Core/Timer.hpp"
#include <string>
#include <stdlib.h>
#include <cstring>
#include <ranges>
#include <unordered_map>

namespace
{

class FILEGuard
{
  FILE* m_f{ nullptr };

public:
  FILEGuard(FILE* f)
    : m_f(f)
  {
  }

  FILEGuard(const FILEGuard&) = delete;
  FILEGuard& operator=(const FILEGuard&) = delete;

  FILEGuard(FILEGuard&& other) noexcept
    : m_f(other.m_f)
  {
    other.m_f = nullptr;
  }
  FILEGuard& operator=(FILEGuard&& other) noexcept
  {
    if (this != &other)
    {
      if (m_f && m_f != stdout && m_f != stderr)
      {
        fflush(m_f);
        fclose(m_f);
      }
      m_f = other.m_f;
      other.m_f = nullptr;
    }
    return *this;
  }

  FILE* operator*() const
  {
    return m_f;
  }

  ~FILEGuard()
  {
    if (m_f && m_f != stdout && m_f != stderr)
    {
      fflush(m_f);
      fclose(m_f);
    }
  }
};

std::unordered_map<std::string, FILEGuard> g_fileMap;

} // namespace

namespace VGG::layer
{
FILE* getLogStream(const char* category)
{
  /*
   * var should be a string of the form:
   * category:stream
   *
   * for category with prefix 'log', default stream is stderr
   * other categories streamed to file with name category_<timestamp>.log
   */
  const char* var = std::getenv("VGG_LAYER_LOG_CONFIG");

  if (!var)
  {
    return stderr;
  }

  if (std::strcmp(var, "stderr") == 0)
    return stderr;
  else if (std::strcmp(var, "stdout") == 0)
    return stdout;
  else if (std::strcmp(var, "file") == 0)
  {
    auto it = g_fileMap.find(category);
    if (it == g_fileMap.end())
    {
      auto filename = std::string(category) + "_" + std::to_string(Timer::now().time) + ".log";
      auto res =
        g_fileMap.emplace(std::make_pair(category, FILEGuard(fopen(filename.c_str(), "w"))));
      if (res.second)
        it = res.first;
    }
    if (it != g_fileMap.end())
    {
      return *(it->second);
    }
  }
  return stderr;
}
} // namespace VGG::layer
