#pragma once

#include "Domain/Saver.hpp"

namespace VGG
{
namespace Model
{

class DirSaver : public Saver
{
  std::string m_model_dir;

public:
  DirSaver(const std::string& modelDir);

  virtual void visit(const std::string& path, const std::vector<char>& content) override;
  // todo, save to same dir, skip write file if not dirty
};

} // namespace Model
} // namespace VGG