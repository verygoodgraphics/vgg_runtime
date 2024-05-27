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

#include <string>
#include <vector>
#include "Domain/Saver.hpp"

namespace VGG
{
namespace Model
{

class DirSaver : public Saver
{
  std::string m_model_dir;

public:
  DirSaver(const std::string& modelDir);

  virtual void visit(const std::string& path, const std::vector<char>& content) override;
  // todo, save to same dir, skip write file if not dirty
};

} // namespace Model
} // namespace VGG
