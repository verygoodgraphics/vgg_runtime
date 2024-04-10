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
#include "ZipLoader.hpp"

#include <zip.h>

#include "Utility/Log.hpp"

namespace VGG
{
namespace Model
{

ZipLoader::ZipLoader(const std::string& filePath)
{
  m_zipFile = zip_open(filePath.c_str(), 0, 'r');
}

ZipLoader::ZipLoader(std::vector<char>& buffer)
{
  m_zipBuffer = std::move(buffer);
  m_zipFile = zip_stream_open(m_zipBuffer.data(), m_zipBuffer.size(), 0, 'r');
}

ZipLoader::~ZipLoader()
{
  if (m_zipFile)
  {
    zip_close(m_zipFile);
    m_zipFile = nullptr;
  }
}

bool ZipLoader::readFile(const std::string& name, std::string& content) const
{
  content.clear();

  if (0 == zip_entry_open(m_zipFile, name.c_str()))
  {
    auto size = zip_entry_size(m_zipFile);
    if (size > 0)
    {
      content.resize(size);
      zip_entry_noallocread(m_zipFile, static_cast<void*>(content.data()), size);
    }
    zip_entry_close(m_zipFile);

    return true;
  }

  DEBUG("#ZipLoader::readFile(), read file failed, %s", name.c_str());
  return false;
}

Loader::ResourcesType ZipLoader::resources() const
{
  ResourcesType resources;

  int n = zip_entries_total(m_zipFile);
  for (auto i = 0; i < n; ++i)
  {
    zip_entry_openbyindex(m_zipFile, i);
    {
      if (zip_entry_isdir(m_zipFile))
      {
        continue;
      }

      std::string fileName{ zip_entry_name(m_zipFile) };
      if (fileName.rfind(K_RESOURCES_DIR_WITH_SLASH, 0) == 0)
      {
        auto size = zip_entry_size(m_zipFile);
        if (size > 0)
        {
          std::vector<char> content;
          content.resize(size);
          zip_entry_noallocread(m_zipFile, static_cast<void*>(content.data()), size);
          resources[fileName] = std::move(content);
        }
        zip_entry_close(m_zipFile);
      }
    }
    zip_entry_close(m_zipFile);
  }

  return resources;
}

} // namespace Model
} // namespace VGG
