#pragma once

#include "JsonDocument.hpp"

#include "Loader/Loader.hpp"
#include "ModelEvent.hpp"

#include "nlohmann/json.hpp"
#include "rxcpp/rx.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct zip_t;

using MakeJsonDocFn = std::function<JsonDocumentPtr(const json&)>;

class VggWork
{
  std::shared_ptr<VGG::Model::Loader> m_loader;

  std::vector<char> m_zip_buffer;
  zip_t* m_zipFile{ nullptr };
  std::unordered_map<std::string, std::string> m_memory_code; // file_name: code_content
  json m_event_listeners;

  JsonDocumentPtr m_designDoc;
  MakeJsonDocFn m_makeDesignDocFn;
  // JsonDocumentPtr m_layoutDoc;
  // MakeJsonDocFn m_makeLayoutDocFn;

  rxcpp::subjects::subject<VGG::ModelEventPtr> m_subject;
  std::mutex m_mutex;

public:
  using ListenersType = std::unordered_map<std::string, std::vector<std::string>>;

  VggWork(const MakeJsonDocFn& makeDesignDocFn);
  ~VggWork();

  bool load(const std::string& filePath);
  bool load(std::vector<char>& buffer);

  const json& codeMapDoc() const;
  JsonDocumentPtr& designDoc();
  const json& layoutDoc() const;

  // event listener
  void addEventListener(const std::string& json_pointer,
                        const std::string& type,
                        const std::string& code);
  void removeEventListener(const std::string& json_pointer,
                           const std::string& type,
                           const std::string& code);
  auto getEventListeners(const std::string& json_pointer) -> ListenersType;

  // observable
  rxcpp::observable<VGG::ModelEventPtr> getObservable();

private:
  bool load();

  bool load(zip_t* zipFile);
  bool readZipFileEntry(zip_t* zipFile, const std::string& entryName, std::string& content) const;

  std::string get_code(const std::string& file_name);

  std::string uuid_for(const std::string& content);
};
