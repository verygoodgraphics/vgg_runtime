#pragma once

#include "Domain/SchemaValidJsonDocument.hpp"
#include "Domain/UndoRedoJsonDocument.hpp"
#include "Domain/Daruma.hpp"

#include <string>
#include <vector>

namespace VGG
{

class EditModel
{
  const std::string& m_design_schema_file_path;
  std::shared_ptr<Daruma> m_model;

public:
  EditModel(const std::string& design_schema_file_path)
    : m_design_schema_file_path{ design_schema_file_path }
  {
    initModel();
  }

  std::shared_ptr<Daruma> open(const std::string& filePath)
  {
    if (m_model->load(filePath))
    {
      return m_model;
    }
    else
    {
      return nullptr;
    }
  }

  bool open(std::vector<char>& buffer);

  std::shared_ptr<Daruma> model()
  {
    return m_model;
  }

private:
  void initModel()
  {
    auto build_design_doc_fn =
      [&, design_schema_file_path = m_design_schema_file_path](const json& design_json)
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

      return JsonDocumentPtr(json_doc_ptr);
    };

    // todo, build layout doc
    m_model.reset(new Daruma(build_design_doc_fn));
  }

  JsonDocument* createJsonDoc()
  {
    return new UndoRedoJsonDocument();
  }
};

} // namespace VGG