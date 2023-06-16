#include "ZipSaver.hpp"

#include "zip.h"

#include <fstream>

namespace fs = std::filesystem;

namespace VGG
{
namespace Model
{

ZipSaver::ZipSaver(const std::string& filePath)
  : m_file_path{ filePath }
{
  std::filesystem::path dirs{ m_file_path };
  dirs.remove_filename();
  fs::create_directories(dirs);

  m_zip_file = zip_open(m_file_path.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
}

ZipSaver::~ZipSaver()
{
  if (m_zip_file)
  {
    zip_close(m_zip_file);
    m_zip_file = nullptr;
  }
}

void ZipSaver::accept(const std::string& path, const std::vector<char>& content)
{
  zip_entry_open(m_zip_file, path.c_str());
  {
    zip_entry_write(m_zip_file, content.data(), content.size());
  }
  zip_entry_close(m_zip_file);
}

} // namespace Model
} // namespace VGG