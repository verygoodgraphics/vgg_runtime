#include "Controller.hpp"

#include "Domain/VggExec.hpp"
#include "Domain/RawJsonDocument.hpp"
#include "Domain/SchemaValidJsonDocument.hpp"
// #include "Domain/UndoRedoJsonDocument.hpp"
#include "Domain/Daruma.hpp"
#include "Presenter.hpp"
#include "Utils/DIContainer.hpp"

#include <cassert>

namespace VGG
{

Controller::Controller(std::shared_ptr<RunLoop> runLoop,
                       std::shared_ptr<Presenter> presenter,
                       RunMode mode)
  : m_run_loop(runLoop)
  , m_presenter(presenter)
  , m_mode(mode)
{
  assert(m_run_loop);
}

bool Controller::start(const std::string& filePath, const char* designDocSchemaFilePath)
{
  initModel(designDocSchemaFilePath);
  auto ret = m_model->load(filePath);
  if (ret)
  {
    start();
  }
  return ret;
}

bool Controller::start(std::vector<char>& buffer, const char* designDocSchemaFilePath)
{
  initModel(designDocSchemaFilePath);
  auto ret = m_model->load(buffer);
  if (ret)
  {
    start();
  }
  return ret;
}

void Controller::initModel(const char* designDocSchemaFilePath)
{
  std::string design_schema_file_path;
  if (designDocSchemaFilePath)
  {
    design_schema_file_path.append(designDocSchemaFilePath);
  }

  auto build_design_doc_fn = [&, design_schema_file_path](const json& design_json)
  {
    auto json_doc_ptr = createJsonDoc();
    json_doc_ptr->setContent(design_json);

    if (!design_schema_file_path.empty())
    {
      SchemaValidJsonDocument::ValidatorPtr design_doc_validator;
      std::ifstream schema_fs(design_schema_file_path);
      json schema = json::parse(schema_fs);
      design_doc_validator.reset(new JsonSchemaValidator);
      design_doc_validator->setRootSchema(schema);

      json_doc_ptr =
        new SchemaValidJsonDocument(JsonDocumentPtr(json_doc_ptr), design_doc_validator);
    }

    return wrapJsonDoc(JsonDocumentPtr(json_doc_ptr));
  };
  // todo, build layout doc
  m_model.reset(new Daruma(build_design_doc_fn));

  VGG::DIContainer<std::shared_ptr<Daruma>>::get() = m_model;
}

void Controller::start()
{
  m_presenter->setModel(m_model);

  observeModelState();
  observeUIEvent();
}

void Controller::observeModelState()
{
  m_model->getObservable()
    .observe_on(m_run_loop->thread())
    .subscribe(m_presenter->getModelObserver());
}

void Controller::observeUIEvent()
{
  auto weak_this = weak_from_this();
  auto observer = rxcpp::make_observer_dynamic<UIEventPtr>(
    [weak_this](UIEventPtr evt)
    {
      auto shared_this = weak_this.lock();
      if (!shared_this)
      {
        return;
      }

      auto listeners_map = shared_this->m_model->getEventListeners(evt->path());
      std::string type = evt->type();
      if (auto it = listeners_map.find(type); it != listeners_map.end())
      {
        for (auto& listener : it->second)
        {
          // todo, evt phase // kCapturingPhase = 1, // kAtTarget = 2, // kBubblingPhase = 3
          // todo, evt PropagationStopped
          shared_this->vggExec()->evalModule(listener, evt);
        }
      }
    });

  // todo, subscribe on new thread to not block current thread while evaluating js code
  m_presenter->getObservable().subscribe(observer);
}

const std::shared_ptr<VggExec>& Controller::vggExec()
{
  return VGG::DIContainer<std::shared_ptr<VggExec>>::get();
}

JsonDocument* Controller::createJsonDoc()
{
  if (m_mode == RunMode::NormalMode)
  {
    return new RawJsonDocument();
  }
  else
  {
    return nullptr; // new UndoRedoJsonDocument();
  }
}

JsonDocumentPtr Controller::wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc)
{
  if (m_mode == RunMode::NormalMode)
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
