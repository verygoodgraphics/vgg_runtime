#include "ZipLoader.hpp"

#include "zip.h"

#include "Log.h"

namespace VGG
{
namespace Model
{

ZipLoader::ZipLoader(const std::string& filePath)
{
  m_zip_file = zip_open(filePath.c_str(), 0, 'r');
}

ZipLoader::ZipLoader(std::vector<char>& buffer)
{
  m_zip_buffer = std::move(buffer);
  m_zip_file = zip_stream_open(m_zip_buffer.data(), m_zip_buffer.size(), 0, 'r');
}

ZipLoader::~ZipLoader()
{
  if (m_zip_file)
  {
    zip_close(m_zip_file);
    m_zip_file = nullptr;
  }
}

bool ZipLoader::readFile(const std::string& name, std::string& content) const
{
  content.clear();

  if (0 == zip_entry_open(m_zip_file, name.c_str()))
  {
    void* buf = NULL;
    size_t bufsize;
    zip_entry_read(m_zip_file, &buf, &bufsize);
    zip_entry_close(m_zip_file);

    content.append(static_cast<char*>(buf), bufsize);
    free(buf);

    return true;
  }

  WARN("#ZipLoader::readFile(), read file failed, %s", name.c_str());
  return false;
}

Loader::ResourcesType ZipLoader::resources() const
{
  ResourcesType resources;

  int n = zip_entries_total(m_zip_file);
  for (auto i = 0; i < n; ++i)
  {
    zip_entry_openbyindex(m_zip_file, i);
    {
      if (zip_entry_isdir(m_zip_file))
      {
        continue;
      }

      std::string file_name{ zip_entry_name(m_zip_file) };
      if (file_name.rfind(ResourcesDirWithSlash, 0) == 0)
      {
        void* buf = NULL;
        size_t bufsize;
        zip_entry_read(m_zip_file, &buf, &bufsize);
        zip_entry_close(m_zip_file);

        char* p_char = static_cast<char*>(buf);
        std::vector<char> content{ p_char, p_char + bufsize };
        resources[file_name] = std::move(content);

        free(buf);
      }
    }
    zip_entry_close(m_zip_file);
  }

  return resources;
}

} // namespace Model
} // namespace VGG