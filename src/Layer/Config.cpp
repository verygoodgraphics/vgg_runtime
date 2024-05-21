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
#include <mutex>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <ranges>
#include <unordered_map>
#include <iostream>
#include <algorithm>

namespace
{

using FILEPtr = std::shared_ptr<FILE>;
std::vector<std::pair<std::string_view, std::string_view>> parse(const char* var)
{
  /*
   * var should be a string of the form:
   * category:stream[;category:stream]
   *
   * for category with prefix 'log', default stream is stderr
   * other categories streamed to file with name <stream>.log
   * stream can be 'stderr', 'stdout', 'null' or a file name
   */
  std::vector<std::pair<std::string_view, std::string_view>> result;
  result.reserve(4);
  std::string_view inputSV(var);

  auto categoryStreamPairs = inputSV | std::views::split(';');
  for (auto&& pair : categoryStreamPairs)
  {
    std::vector<std::string_view> categoryStream;
    for (auto&& part : pair | std::views::split(':'))
    {
      categoryStream.emplace_back(&*part.begin(), std::ranges::distance(part));
    }
    if (categoryStream.size() == 2)
    {
      result.emplace_back(categoryStream[0], categoryStream[1]);
    }
  }
  return result;
}

struct ConfigVars
{
  std::once_flag flag;

  std::vector<std::pair<std::string_view, std::string_view>> logConfigs;
  bool                                                       enableLog{ false };
  ConfigVars()
  {
    std::call_once(
      flag,
      [this]()
      {
        const char* var = std::getenv("VGG_LAYER_LOG");
        if (var)
        {
          logConfigs = parse(var);
          std::sort(logConfigs.begin(), logConfigs.end());
          logConfigs.erase(std::unique(logConfigs.begin(), logConfigs.end()), logConfigs.end());
        }

        const char* enableLogVar = std::getenv("VGG_ENABLE_LAYER_LOG");
        if (enableLogVar)
        {
          if (strcmp(enableLogVar, "0") == 0)
          {
            enableLog = false;
          }
          else if (strcmp(enableLogVar, "1") == 0)
          {
            enableLog = true;
          }
        }

        if (enableLog)
        {
          for (const auto& [cat, stream] : logConfigs)
          {
            std::cout << "Log category: " << cat << " stream to " << stream << std::endl;
          }
        }
      });
  }
};

ConfigVars                               g_configVars;
std::unordered_map<std::string, FILEPtr> g_categoryMap; // category -> FILEPtr
std::unordered_map<std::string, FILEPtr> g_file;        // string -> FILEPtr

} // namespace

namespace VGG::layer
{

FILE* getLogStream(const char* category)
{
  if (!g_configVars.enableLog)
  {
    return nullptr;
  }
  if (!category)
  {
    return stderr;
  }
  namespace sv = std::views;
  auto it = g_categoryMap.find(category);
  if (it == g_categoryMap.end())
  {
    for (const auto& [cat, stream] : g_configVars.logConfigs)
    {
      if (cat == category)
      {
        std::pair<std::unordered_map<std::string, FILEPtr>::iterator, bool> res;
        if (stream == "stderr")
        {
          res = g_categoryMap.emplace(std::make_pair(category, FILEPtr(stderr, [](FILE*) {})));
        }
        else if (stream == "stdout")
        {
          res = g_categoryMap.emplace(std::make_pair(category, FILEPtr(stdout, [](FILE*) {})));
        }
        else if (stream == "null")
        {
          res = g_categoryMap.emplace(std::make_pair(category, nullptr));
        }
        else
        {
          auto streamItr = g_file.find(std::string(stream));
          if (streamItr == g_file.end())
          {
            FILE* f = fopen(std::string(stream).c_str(), "w");
            if (f)
            {
              FILEPtr fp(f, [](FILE* f) {});
              g_file.emplace(std::make_pair(stream, std::move(fp)));
              res = g_categoryMap.emplace(std::make_pair(category, fp));
            }
            else
            {
              res = g_categoryMap.emplace(std::make_pair(category, stderr));
            }
          }
          else
          {
            res = g_categoryMap.emplace(std::make_pair(category, streamItr->second));
          }
        }
        // ASSERT(res.second);
        return res.first->second.get();
      }
    }
  }
  if (it != g_categoryMap.end())
  {
    return it->second.get();
  }
  return stderr;
}

} // namespace VGG::layer
