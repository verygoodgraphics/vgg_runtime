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

#include "Utility/VggString.hpp"

#include <numeric>

namespace VGG::Helper
{

std::string join(const std::vector<std::string>& instanceIdStack, const std::string& separator)
{
  if (instanceIdStack.empty())
  {
    return {};
  }

  return std::accumulate(
    std::next(instanceIdStack.begin()),
    instanceIdStack.end(),
    instanceIdStack[0], // start with first element
    [&](const std::string& a, const std::string& b) { return a + separator + b; });
}

std::vector<std::string> split(const std::string& s, const std::string& separator)
{
  size_t                   pos_start = 0, pos_end, delim_len = separator.length();
  std::string              token;
  std::vector<std::string> res;

  while ((pos_end = s.find(separator, pos_start)) != std::string::npos)
  {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

} // namespace VGG::Helper