
#include "ZipLoader.hpp"

#include "zip.h"

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
  m_zip_buffer = std::move(buffer);
  m_zipFile = zip_stream_open(m_zip_buffer.data(), m_zip_buffer.size(), 0, 'r');
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
    void* buf = NULL;
    size_t bufsize;
    zip_entry_read(m_zipFile, &buf, &bufsize);
    zip_entry_close(m_zipFile);

    content.append(static_cast<char*>(buf), bufsize);
    free(buf);

    return true;
  }

  return false;
}

} // namespace Model
} // namespace VGG