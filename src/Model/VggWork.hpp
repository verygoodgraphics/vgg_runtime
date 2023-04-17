#pragma once

#include "JsonDocument.hpp"

#include "nlohmann/json.hpp"

#include <functional>
#include <memory>
#include <string>

namespace miniz_cpp
{
class zip_file;
}

using MakeJsonDocFn = std::function<JsonDocumentPtr(const json&)>;

class VggWork
{
public:
  VggWork(const MakeJsonDocFn& makeDesignDocFn);
  ~VggWork() = default;

  bool load(const std::string& filePath);
  bool load(const std::vector<unsigned char>& buffer);

  const json& codeMapDoc() const;
  const json& designDoc() const;
  const json& layoutDoc() const;

  const std::string getCode(const std::string& path) const;

  void createCode(const json::json_pointer& path, const std::string& name);
  void deleteCode(const json::json_pointer& path);
  void updateCode(const std::string& name, const std::string& code);

private:
  using LoadZipFn = std::function<void(miniz_cpp::zip_file&)>;
  bool loadTemplate(LoadZipFn fn);
  bool load(miniz_cpp::zip_file& zipFile);

  const std::string& codeName(const json::json_pointer& path) const;
  const std::string& jsCode(const std::string& name) const;

  bool readZipFileEntry(miniz_cpp::zip_file& zipFile,
                        const std::string& entryName,
                        std::string& content) const;

  std::shared_ptr<miniz_cpp::zip_file> m_zipFile;

  JsonDocumentPtr m_designDoc;
  MakeJsonDocFn m_makeDesignDocFn;

  std::unordered_map<std::string, std::string> m_codeMap;

  // JsonDocumentPtr m_layoutDoc;
  // MakeJsonDocFn m_makeLayoutDocFn;
};
