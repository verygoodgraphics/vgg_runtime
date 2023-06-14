#pragma once

#include "Loader.hpp"

#include <string>
#include <vector>

struct zip_t;

namespace VGG
{
namespace Model
{

class ZipLoader : public Loader
{
  std::vector<char> m_zip_buffer;
  zip_t* m_zipFile{ nullptr };

public:
  ZipLoader(const std::string& filePath);
  ZipLoader(std::vector<char>& buffer);
  virtual ~ZipLoader();

  virtual bool readFile(const std::string& name, std::string& content) const override;

private:
  bool load();
};

} // namespace Model
} // namespace VGG