#pragma once

#include <Application/AppRenderable.h>
#include <Application/AppScene.h>
#include <Application/RunLoop.hpp>

#include <Domain/Layout/Layout.hpp>
#include <Domain/Layout/Rect.hpp>

#include <rxcpp/rx.hpp>

#include <memory>
#include <string>

class VggExec;
class JsonDocument;

namespace VGG
{

class Daruma;
class Editor;
class Presenter;
class ViewModel;

class Controller : public std::enable_shared_from_this<Controller>
{
public:
  enum class ERunMode
  {
    NORMAL_MODE,
    EDIT_MODE
  };

private:
  std::string m_designSchemaFilePath;
  std::shared_ptr<RunLoop> m_runLoop;
  std::shared_ptr<Presenter> m_presenter;
  std::shared_ptr<Editor> m_editor;

  ERunMode m_mode;
  std::shared_ptr<Daruma> m_model;
  std::shared_ptr<Daruma> m_editModel;
  std::shared_ptr<Layout::Layout> m_layout;

public:
  Controller(std::shared_ptr<RunLoop> runLoop,
             std::shared_ptr<Presenter> presenter,
             std::shared_ptr<Editor> editor,
             ERunMode mode = ERunMode::NORMAL_MODE);
  ~Controller() = default;

  bool start(const std::string& filePath,
             const char* designDocSchemaFilePath = nullptr,
             const char* layoutDocSchemaFilePath = nullptr);
  bool start(std::vector<char>& buffer,
             const char* designDocSchemaFilePath = nullptr,
             const char* layoutDocSchemaFilePath = nullptr);

  bool edit(const std::string& filePath);
  bool edit(std::vector<char>& buffer);

  void onResize();

  std::shared_ptr<app::AppRenderable> editor();
  void setEditMode(bool editMode);
  bool isEditMode()
  {
    return m_mode == ERunMode::EDIT_MODE;
  }

  bool hasDirtyEditor();
  void resetEditorDirty();

private:
  void initModel(const char* designDocSchemaFilePath, const char* layoutDocSchemaFilePath);
  JsonDocument* createJsonDoc();
  std::shared_ptr<JsonDocument> wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc);
  const std::shared_ptr<VggExec>& vggExec();

  void start();
  void observeModelState();
  void observeViewEvent();

  void startEditing();
  void observeEditModelState();
  void observeEditViewEvent();

  std::shared_ptr<ViewModel> generateViewModel(std::shared_ptr<Daruma> model, Layout::Size size);
  MakeJsonDocFn createMakeJsonDocFn(const char* pJsonSchemaFilePath);
};

} // namespace VGG