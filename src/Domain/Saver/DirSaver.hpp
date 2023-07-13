#pragma once

#include "Domain/Saver.hpp"

namespace VGG
{
namespace Model
{

class DirSaver : public Saver
{
  std::string m_work_dir;

public:
  DirSaver(const std::string& workDir);

  virtual void accept(const std::string& path, const std::vector<char>& content) override;
  // todo, save to same dir, skip write file if not dirty
};

} // namespace Model
} // namespace VGG