#include "DirSaver.hpp"

#include "Utils/Utils.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace VGG
{
namespace Model
{

DirSaver::DirSaver(const std::string& workDir)
  : m_work_dir{ workDir }
{
  // todo, check if same as src dir
}

void DirSaver::accept(const std::string& path, const std::vector<char>& content)
{
  std::filesystem::path file_path{ m_work_dir };
  file_path /= path;

  auto dirs{ file_path };
  dirs.remove_filename();
  fs::create_directories(dirs);

  std::ofstream ofs{ file_path, std::ios::binary };
  if (ofs.is_open())
  {
    ofs.write(content.data(), content.size());
    ofs.close();
  }
  else
  {
    FAIL("#DirSaver::accept, unable to open file: %s, %s", file_path.c_str(), std::strerror(errno));
  }
}

} // namespace Model
} // namespace VGG
