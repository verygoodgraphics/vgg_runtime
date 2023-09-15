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
    auto size = zip_entry_size(m_zip_file);
    if (size > 0)
    {
      content.resize(size);
      zip_entry_noallocread(m_zip_file, static_cast<void*>(content.data()), size);
    }
    zip_entry_close(m_zip_file);

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

      std::string fileName{ zip_entry_name(m_zip_file) };
      if (fileName.rfind(ResourcesDirWithSlash, 0) == 0)
      {
        auto size = zip_entry_size(m_zip_file);
        if (size > 0)
        {
          std::vector<char> content;
          content.resize(size);
          zip_entry_noallocread(m_zip_file, static_cast<void*>(content.data()), size);
          resources[fileName] = std::move(content);
        }
        zip_entry_close(m_zip_file);
      }
    }
    zip_entry_close(m_zip_file);
  }

  return resources;
}

} // namespace Model
} // namespace VGG