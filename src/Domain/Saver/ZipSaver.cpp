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
#include "ZipSaver.hpp"

#include <zip.h>

#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

namespace VGG
{
namespace Model
{

ZipSaver::ZipSaver(const std::string& filePath)
  : m_filePath{ filePath }
{
  std::filesystem::path dirs{ m_filePath };
  dirs.remove_filename();
  fs::create_directories(dirs);

  m_zipFile = zip_open(m_filePath.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
}

ZipSaver::~ZipSaver()
{
  if (m_zipFile)
  {
    zip_close(m_zipFile);
    m_zipFile = nullptr;
  }
}

void ZipSaver::visit(const std::string& path, const std::vector<char>& content)
{
  zip_entry_open(m_zipFile, path.c_str());
  {
    zip_entry_write(m_zipFile, content.data(), content.size());
  }
  zip_entry_close(m_zipFile);
}

} // namespace Model
} // namespace VGG
