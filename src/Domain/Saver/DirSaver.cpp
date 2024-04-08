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
#include "DirSaver.hpp"

#include "Utility/Log.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace VGG
{
namespace Model
{

DirSaver::DirSaver(const std::string& modelDir)
  : m_model_dir{ modelDir }
{
  // todo, check if same as src dir
}

void DirSaver::visit(const std::string& path, const std::vector<char>& content)
{
  std::filesystem::path filePath{ m_model_dir };
  filePath /= path;

  auto dirs{ filePath };
  dirs.remove_filename();
  fs::create_directories(dirs);

  std::ofstream ofs{ filePath, std::ios::binary };
  if (ofs.is_open())
  {
    ofs.write(content.data(), content.size());
    ofs.close();
  }
  else
  {
    FAIL("#DirSaver::accept, unable to open file: %s, %s", filePath.c_str(), std::strerror(errno));
  }
}

} // namespace Model
} // namespace VGG
