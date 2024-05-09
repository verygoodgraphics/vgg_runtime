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

#include "Layer/Config.hpp"
#include "Layer/Core/ResourceProvider.hpp"
#include "Layer/LayerCache.h"
#include <core/SkData.h>
#include <string_view>
#include <string>
#include <fstream>
#include <filesystem>

namespace VGG::layer
{

class FileResourceProvider : public ResourceProvider
{
public:
  FileResourceProvider(std::filesystem::path cwd)
    : m_cwd(cwd)
  {
  }
  FileResourceProvider() = default;
  FileResourceProvider(const FileResourceProvider&) = delete;
  FileResourceProvider& operator=(const FileResourceProvider&) = delete;

  FileResourceProvider(FileResourceProvider&&) = default;
  FileResourceProvider& operator=(FileResourceProvider&&) = default;

  void setDirPrefix(std::filesystem::path cwd)
  {
    m_cwd = cwd;
  }

  Blob readData(std::string_view guid) override
  {
    auto          filename = m_cwd / guid;
    std::ifstream in(filename, std::ios::binary);
    if (in.is_open() == false)
    {
      VGG_LAYER_LOG("cannot open {}", filename.string());
      return nullptr;
    }
    in.seekg(0, std::ios::end);
    const auto length = in.tellg();
    in.seekg(0, std::ios::beg);
    char* data = (char*)malloc(length);
    if (!in.read(data, length))
    {
      VGG_LAYER_LOG("read data failed");
      return nullptr;
    }
    return SkData::MakeFromMalloc(data, length);
  }

private:
  std::filesystem::path m_cwd;
};
} // namespace VGG::layer
