#include "Controller.hpp"

#include "Exec/VggExec.hpp"
#include "Model/RawJsonDocument.hpp"
#include "Model/SchemaValidJsonDocument.hpp"
#include "Model/SubjectJsonDocument.hpp"
#include "Model/UndoRedoJsonDocument.hpp"
#include "Model/VggWork.hpp"
#include "Presenter/Presenter.hpp"
#include "Utils/DIContainer.hpp"

#include <cassert>

namespace VGG
{

Controller::Controller(std::shared_ptr<RunLoop> runLoop, Presenter& presenter, RunMode mode)
  : m_run_loop(runLoop)
  , m_design_doc_observer(presenter.getDesignDocObserver())
  , m_layout_doc_observer(presenter.getLayoutDocObserver())
  , m_mode(mode)
{
  assert(m_run_loop);

  m_view_event_observer = rxcpp::make_observer_dynamic<UIEventPtr>(
    [&](UIEventPtr evt)
    {
      auto listenters = m_work->getEventListeners(evt->path, ViewEventTypeToString(evt->type));
      for (auto& listenter : listenters)
      {
        // todo, pass evt data to listeners
        vggExec()->evalModule(listenter);
      }
    });
  presenter.getObservable().subscribe(m_view_event_observer);
}

bool Controller::start(const std::string& filePath, const char* designDocSchemaFilePath)
{
  initVggWork(designDocSchemaFilePath);
  return m_work->load(filePath);
}

bool Controller::start(const std::vector<char>& buffer, const char* designDocSchemaFilePath)
{
  initVggWork(designDocSchemaFilePath);
  return m_work->load(buffer);
}

void Controller::initVggWork(const char* designDocSchemaFilePath)
{
  std::string design_schema_file_path;
  if (designDocSchemaFilePath)
  {
    design_schema_file_path.append(designDocSchemaFilePath);
  }
  auto build_design_doc_fn = [&, design_schema_file_path](const json& design_json)
  {
    auto json_doc_raw_ptr = createJsonDoc();
    json_doc_raw_ptr->setContent(design_json);

    if (!design_schema_file_path.empty())
    {
      SchemaValidJsonDocument::ValidatorPtr design_doc_validator;
      std::ifstream schema_fs(design_schema_file_path);
      json schema = json::parse(schema_fs);
      design_doc_validator.reset(new JsonSchemaValidator);
      design_doc_validator->setRootSchema(schema);

      json_doc_raw_ptr =
        new SchemaValidJsonDocument(JsonDocumentPtr(json_doc_raw_ptr), design_doc_validator);
    }

    auto subject_doc = new SubjectJsonDocument(JsonDocumentPtr(json_doc_raw_ptr));
    subject_doc->getObservable().observe_on(m_run_loop->thread()).subscribe(m_design_doc_observer);

    return wrapJsonDoc(JsonDocumentPtr(subject_doc));
  };
  // todo, build layout doc
  m_work.reset(new VggWork(build_design_doc_fn));

  VGG::DIContainer<std::shared_ptr<VggWork>>::get() = m_work;
}

const std::shared_ptr<VggExec>& Controller::vggExec()
{
  return VGG::DIContainer<std::shared_ptr<VggExec>>::get();
}

JsonDocument* Controller::createJsonDoc()
{
  if (m_mode == NormalMode)
  {
    return new RawJsonDocument();
  }
  else
  {
    return new UndoRedoJsonDocument();
  }
}

JsonDocumentPtr Controller::wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc)
{
  if (m_mode == NormalMode)
  {
    return jsonDoc;
  }
  else
  {
    // todo, wrap with remote doc which can save to server
    return jsonDoc;
  }
}

} // namespace VGG