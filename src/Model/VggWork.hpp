#pragma once

#include "nlohmann/json.hpp"

#include <string>
#include <memory>

class DocumentModel;

namespace miniz_cpp
{
class zip_file;
}

class VggWork
{
public:
  using json = nlohmann::json;

  virtual ~VggWork() = default;

  bool load(const std::string& filePath);
  bool load(const std::vector<unsigned char>& buffer);

  const json& codeMap() const;
  const json& artboard() const;
  const json& layout() const;

  const std::string& getCode(const json::json_pointer& path) const;
  void createCode(const json::json_pointer& path, const std::string& name);
  void deleteCode(const json::json_pointer& path);
  void updateCode(const std::string& name, const std::string& code);

private:
  bool load(miniz_cpp::zip_file& zipFile);

  const std::string& codeName(const json::json_pointer& path) const;
  const std::string& jsCode(const std::string& name) const;

  std::shared_ptr<DocumentModel> m_artboard;
};
