#pragma once

#include "JsonDocument.hpp"

#include "nlohmann/json.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct zip_t;

using MakeJsonDocFn = std::function<JsonDocumentPtr(const json&)>;

class VggWork
{
  zip_t* m_zipFile{ nullptr };
  std::unordered_map<std::string, std::string> m_memory_code; // file_name: code_content
  json m_event_listeners;

  JsonDocumentPtr m_designDoc;
  MakeJsonDocFn m_makeDesignDocFn;
  // JsonDocumentPtr m_layoutDoc;
  // MakeJsonDocFn m_makeLayoutDocFn;

  std::unordered_map<std::string, std::string> m_codeMap;

public:
  VggWork(const MakeJsonDocFn& makeDesignDocFn);
  ~VggWork();

  bool load(const std::string& filePath);
  bool load(const std::vector<char>& buffer);

  const json& codeMapDoc() const;
  JsonDocumentPtr& designDoc();
  const json& layoutDoc() const;

  const std::string getCode(const std::string& path) const;

  void addEventListener(const std::string& json_pointer,
                        const std::string& type,
                        const std::string& code);
  void removeEventListener(const std::string& json_pointer,
                           const std::string& type,
                           const std::string& code);
  const std::vector<std::string> getEventListeners(const std::string& json_pointer,
                                                   const std::string& type);

private:
  bool load(zip_t* zipFile);
  bool readZipFileEntry(zip_t* zipFile, const std::string& entryName, std::string& content) const;

  std::string get_code(const std::string& file_name);

  std::string uuid_for(const std::string& content);
};
