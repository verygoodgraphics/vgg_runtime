#pragma once

#include "RunLoop.hpp"

#include "rxcpp/rx.hpp"

#include <memory>
#include <string>

class VggExec;
class JsonDocument;

namespace VGG
{

class Daruma;
class Presenter;

class Controller : public std::enable_shared_from_this<Controller>
{
  std::string m_design_schema_file_path;

public:
  enum class RunMode
  {
    NormalMode,
    EditMode
  };

  Controller(std::shared_ptr<RunLoop> runLoop,
             std::shared_ptr<Presenter> presenter,
             RunMode mode = RunMode::NormalMode);
  ~Controller() = default;

  bool start(const std::string& filePath, const char* designDocSchemaFilePath = nullptr);
  bool start(std::vector<char>& buffer, const char* designDocSchemaFilePath = nullptr);

  bool edit(const std::string& filePath);
  bool edit(std::vector<char>& buffer);

private:
  std::shared_ptr<RunLoop> m_run_loop;
  std::shared_ptr<Presenter> m_presenter;

  RunMode m_mode;
  std::shared_ptr<Daruma> m_model;
  std::shared_ptr<Daruma> m_edit_model;

private:
  void initModel(const char* designDocSchemaFilePath);
  JsonDocument* createJsonDoc();
  std::shared_ptr<JsonDocument> wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc);
  const std::shared_ptr<VggExec>& vggExec();

  void start();
  void observeModelState();
  void observeUIEvent();

  void startEditing();
};

} // namespace VGG