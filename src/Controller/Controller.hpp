#pragma once

#include "Model/SubjectJsonDocument.hpp"

#include <memory>
#include <string>

class VggWork;
class VggExec;

class Controller
{
public:
  enum RunMode
  {
    NormalMode,
    EditMode
  };

  Controller(JsonDocumentObserverPtr designDocObsever = JsonDocumentObserverPtr(),
             JsonDocumentObserverPtr layoutDocObsever = JsonDocumentObserverPtr(),
             RunMode mode = NormalMode);
  ~Controller() = default;

  bool start(const std::string& filePath, const char* designDocSchemaFilePath = nullptr);
  bool start(const std::vector<char>& buffer, const char* designDocSchemaFilePath = nullptr);

  void onClick(const std::string& path);

private:
  JsonDocumentObserverPtr m_design_doc_observer;
  JsonDocumentObserverPtr m_layout_doc_observer;
  RunMode m_mode;
  std::shared_ptr<VggWork> m_work;

  void initVggWork(const char* designDocSchemaFilePath);
  JsonDocument* createJsonDoc();
  JsonDocumentPtr wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc);

  const std::shared_ptr<VggExec>& vggExec();
};
