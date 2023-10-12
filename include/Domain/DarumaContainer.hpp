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
#pragma once

#include "Daruma.hpp"

#include <unordered_map>
#include <memory>

namespace VGG
{
class DarumaContainer
{
public:
  enum class KeyType
  {
    MainOrEditor,
    Edited
  };

  void add(std::shared_ptr<Daruma> daruma, KeyType key = KeyType::MainOrEditor);
  void remove(KeyType key = KeyType::MainOrEditor);
  std::shared_ptr<Daruma> get(KeyType key = KeyType::MainOrEditor);

private:
  auto& getRepo()
  {
    static std::unordered_map<KeyType, std::shared_ptr<Daruma>> repo;
    return repo;
  }
};

} // namespace VGG
