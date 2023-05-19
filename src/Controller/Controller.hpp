#pragma once

#include "Model/SubjectJsonDocument.hpp"
#include "PlatformAdapter/Native/RunLoop/RunLoop.hpp"

#include "rxcpp/rx.hpp"

#include <memory>
#include <string>

class VggWork;
class VggExec;

namespace VGG
{

class Controller
{
public:
  enum RunMode
  {
    NormalMode,
    EditMode
  };

  Controller(
    std::shared_ptr<RunLoop> runLoop,
    rxcpp::observer<ModelEventPtr> designDocObsever =
      rxcpp::make_observer_dynamic<ModelEventPtr>(rxcpp::detail::OnNextEmpty<ModelEventPtr>()),
    rxcpp::observer<ModelEventPtr> layoutDocObsever =
      rxcpp::make_observer_dynamic<ModelEventPtr>(rxcpp::detail::OnNextEmpty<ModelEventPtr>()),
    RunMode mode = NormalMode);
  ~Controller() = default;

  bool start(const std::string& filePath, const char* designDocSchemaFilePath = nullptr);
  bool start(const std::vector<char>& buffer, const char* designDocSchemaFilePath = nullptr);

  void onClick(const std::string& path);

private:
  std::shared_ptr<RunLoop> m_run_loop;
  rxcpp::observer<ModelEventPtr> m_design_doc_observer;
  rxcpp::observer<ModelEventPtr> m_layout_doc_observer;
  RunMode m_mode;
  std::shared_ptr<VggWork> m_work;

  void initVggWork(const char* designDocSchemaFilePath);
  JsonDocument* createJsonDoc();
  JsonDocumentPtr wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc);
  const std::shared_ptr<VggExec>& vggExec();
};

}