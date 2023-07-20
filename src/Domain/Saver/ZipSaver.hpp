#pragma once

#include "Domain/Saver.hpp"

struct zip_t;

namespace VGG
{
namespace Model
{

class ZipSaver : public Saver
{
  std::string m_file_path;
  zip_t* m_zip_file{ nullptr };

public:
  ZipSaver(const std::string& filePath);
  ~ZipSaver();

  virtual void visit(const std::string& path, const std::vector<char>& content) override;
};

} // namespace Model
} // namespace VGG