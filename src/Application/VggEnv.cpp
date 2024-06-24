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

#include "Application/VggEnv.hpp"
#include <chrono>
#include <unordered_map>
#include "Utility/Log.hpp"

namespace VGG
{

VggEnv::VggEnv()
{
  using namespace std::chrono;
  m_tag = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::weak_ptr<VggEnv> VggEnv::getDefault()
{
  auto& repo = getRepo();
  ASSERT(!repo.empty());
  return repo.begin()->second;
}

std::weak_ptr<VggEnv> VggEnv::get(const std::string& key)
{
  return getRepo()[key];
}

void VggEnv::set(const std::string& key, std::weak_ptr<VggEnv> env)
{
  getRepo()[key] = env;
}

void VggEnv::erase(const std::string& key)
{
  getRepo().erase(key);
}

std::unordered_map<std::string, std::weak_ptr<VggEnv>>& VggEnv::getRepo()
{
  static std::unordered_map<std::string, std::weak_ptr<VggEnv>> repo;
  return repo;
}

} // namespace VGG