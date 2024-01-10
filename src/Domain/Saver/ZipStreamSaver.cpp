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
#include "ZipStreamSaver.hpp"

#include "Utility/Log.hpp"

#include <zip.h>

namespace VGG
{
namespace Model
{

ZipStreamSaver::ZipStreamSaver()
{
  m_zipFile = zip_stream_open(NULL, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
  ASSERT(m_zipFile);
}

ZipStreamSaver::~ZipStreamSaver()
{
  ASSERT(m_zipFile);
  if (m_zipFile)
  {
    zip_stream_close(m_zipFile);
    m_zipFile = nullptr;
  }
}

void ZipStreamSaver::visit(const std::string& path, const std::vector<char>& content)
{
  ASSERT(m_zipFile);
  if (!m_zipFile)
  {
    return;
  }

  zip_entry_open(m_zipFile, path.c_str());
  {
    zip_entry_write(m_zipFile, content.data(), content.size());
  }
  zip_entry_close(m_zipFile);
}

std::vector<uint8_t> ZipStreamSaver::buffer()
{
  uint8_t*    outBuffer = NULL;
  std::size_t outBufferSize = 0;

  std::vector<uint8_t> result;
  ASSERT(m_zipFile);
  if (m_zipFile)
  {
    zip_stream_copy(m_zipFile, (void**)&outBuffer, &outBufferSize);
    result.insert(result.end(), outBuffer, outBuffer + outBufferSize);
    free(outBuffer);
  }

  return result;
}

} // namespace Model
} // namespace VGG
