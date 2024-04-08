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

#include "Loader.hpp"

#include <string>
#include <vector>

struct zip_t;

namespace VGG
{
namespace Model
{

class ZipLoader : public Loader
{
  std::vector<char> m_zipBuffer;
  zip_t* m_zipFile{ nullptr };

public:
  ZipLoader(const std::string& filePath);
  ZipLoader(std::vector<char>& buffer);
  virtual ~ZipLoader();

  virtual bool readFile(const std::string& name, std::string& content) const override;
  virtual ResourcesType resources() const override;

private:
  bool load();
};

} // namespace Model
} // namespace VGG
