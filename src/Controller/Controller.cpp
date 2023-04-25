#include "Controller.hpp"

#include "Model/RawJsonDocument.hpp"
#include "Model/SchemaValidJsonDocument.hpp"
#include "Model/SubjectJsonDocument.hpp"
#include "Model/UndoRedoJsonDocument.hpp"
#include "Model/VggWork.hpp"
#include "VggDepContainer.hpp"
#include "VggExec.hpp"

Controller::Controller(JsonDocumentObserverPtr designDocObsever,
                       JsonDocumentObserverPtr layoutDocObsever,
                       RunMode mode)
  : m_design_doc_observer(designDocObsever)
  , m_layout_doc_observer(layoutDocObsever)
  , m_mode(mode)
{
}

bool Controller::start(const std::string& filePath, const char* designDocSchemaFilePath)
{
  initVggWork(designDocSchemaFilePath);
  return m_work->load(filePath);
}

bool Controller::start(const std::vector<unsigned char>& buffer,
                       const char* designDocSchemaFilePath)
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
    auto json_doc = createJsonDoc();
    json_doc->setContent(design_json);

    SchemaValidJsonDocument::ValidatorPtr design_doc_validator;
    if (!design_schema_file_path.empty())
    {
      std::ifstream schema_fs(design_schema_file_path);
      json schema = json::parse(schema_fs);
      design_doc_validator.reset(new JsonSchemaValidator);
      design_doc_validator->setRootSchema(schema);
    }
    auto schema_valid_doc =
      new SchemaValidJsonDocument(JsonDocumentPtr(json_doc), design_doc_validator);

    auto subject_doc = new SubjectJsonDocument(JsonDocumentPtr(schema_valid_doc));
    if (m_design_doc_observer)
    {
      subject_doc->addObserver(m_design_doc_observer);
    }

    return wrapJsonDoc(JsonDocumentPtr(subject_doc));
  };
  // todo, build layout doc
  m_work.reset(new VggWork(build_design_doc_fn));

  VggDepContainer<std::shared_ptr<VggWork>>::get() = m_work;
}

void Controller::onClick(const std::string& path)
{
  // todo: create event?
  auto code = m_work->getCode(path);
  vggExec()->evalModule(code);
}

const std::shared_ptr<VggExec>& Controller::vggExec()
{
  return VggDepContainer<std::shared_ptr<VggExec>>::get();
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