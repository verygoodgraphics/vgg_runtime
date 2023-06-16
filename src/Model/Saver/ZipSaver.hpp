#pragma once

#include "Saver.hpp"

#include <string>

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

  virtual void accept(const std::string& path, const std::string& content) override;
};

} // namespace Model
} // namespace VGG