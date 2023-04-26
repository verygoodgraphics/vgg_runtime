#pragma once

#include "JsonDocument.hpp"

#include "nlohmann/json.hpp"

#include <functional>
#include <memory>
#include <string>

struct zip_t;

using MakeJsonDocFn = std::function<JsonDocumentPtr(const json&)>;

class VggWork
{
public:
  VggWork(const MakeJsonDocFn& makeDesignDocFn);
  ~VggWork();

  bool load(const std::string& filePath);
  bool load(const std::vector<char>& buffer);

  const json& codeMapDoc() const;
  JsonDocumentPtr& designDoc();
  const json& layoutDoc() const;

  const std::string getCode(const std::string& path) const;

  void createCode(const json::json_pointer& path, const std::string& name);
  void deleteCode(const json::json_pointer& path);
  void updateCode(const std::string& name, const std::string& code);

private:
  bool load(zip_t* zipFile);

  const std::string& codeName(const json::json_pointer& path) const;
  const std::string& jsCode(const std::string& name) const;

  bool readZipFileEntry(zip_t* zipFile, const std::string& entryName, std::string& content) const;

  zip_t* m_zipFile{ nullptr };

  JsonDocumentPtr m_designDoc;
  MakeJsonDocFn m_makeDesignDocFn;

  std::unordered_map<std::string, std::string> m_codeMap;

  // JsonDocumentPtr m_layoutDoc;
  // MakeJsonDocFn m_makeLayoutDocFn;
};
