#include "DirSaver.hpp"

#include "Utils/Utils.hpp"

#include <cstring>
#include <fstream>
#include <iostream>

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

void DirSaver::accept(const std::string& path, const std::string& content)
{
  std::filesystem::path p{ m_work_dir };
  p /= path;

  auto dirs{ p };
  dirs.remove_filename();
  fs::create_directories(dirs);

  std::ofstream out{ p, std::ios::binary };
  if (out.is_open())
  {
    out << content;
    out.close();
  }
  else
  {
    FAIL("#DirSaver::accept, unable to open file: %s, %s", p.c_str(), std::strerror(errno));
  }
}

} // namespace Model
} // namespace VGG