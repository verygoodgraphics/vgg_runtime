#pragma once

#include "Loader.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace VGG
{
namespace Model
{

class DirLoader : public Loader
{
  std::string m_path;

public:
  DirLoader(const std::string& path)
    : m_path{ path }
  {
  }

  virtual bool readFile(const std::string& name, std::string& content) const override
  {
    content.clear();

    std::filesystem::path dir{ m_path };
    std::ifstream ifs{ dir / name, std::ios::binary };
    if (!ifs)
    {
      return false;
    }
    std::istreambuf_iterator<char> start{ ifs }, end;
    content.append(start, end);
    return true;
  }

  virtual ResourcesType resources() const override
  {
    std::filesystem::path dir{ m_path };
    dir /= ResourcesDirWithSlash;

    ResourcesType resources;

    if (fs::exists(dir) && fs::is_directory(dir))
    {
      for (auto const& dir_entry : fs::recursive_directory_iterator(dir))
      {
        if (dir_entry.is_regular_file())
        {
          auto relative_path = dir_entry.path().lexically_relative(dir);

          std::string key{ ResourcesDir };
          for (auto it = relative_path.begin(); it != relative_path.end(); ++it)
          {
            key.append("/"); // use "/" on both windows & posix
            key.append(it->string());
          }

          std::ifstream ifs{ dir_entry.path(), std::ios::binary };
          std::istreambuf_iterator<char> start{ ifs }, end;
          std::vector<char> content{ start, end };
          resources[key] = std::move(content);
        }
      }
    }

    return resources;
  }
};

} // namespace Model
} // namespace VGG
