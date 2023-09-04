#include "DirSaver.hpp"

#include "Log.h"

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
