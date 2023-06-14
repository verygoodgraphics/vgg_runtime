#pragma once

#include "Loader.hpp"

#include <filesystem>
#include <fstream>
#include <string>

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

  virtual bool load()
  {
    return true;
  }

  virtual bool readFile(const std::string& name, std::string& content) const
  {
    content.clear();

    std::filesystem::path dir{ m_path };
    std::ifstream t{ dir / name };
    content.append(std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>());
    return true;
  }
};

} // namespace Model
} // namespace VGG