/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/RawJsonDocument.hpp"
#include "Domain/SchemaValidJsonDocument.hpp"
#include "Domain/UndoRedoJsonDocument.hpp"
#include "Utility/Log.hpp"

#include <string>
#include <vector>

namespace VGG
{

class EditModel
{
  const std::string& m_designSchemaFilePath;
  std::shared_ptr<Daruma> m_model;

public:
  EditModel(const std::string& design_schema_file_path)
    : m_designSchemaFilePath{ design_schema_file_path }
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

  std::shared_ptr<Daruma> open(std::vector<char>& buffer)
  {
    if (m_model->load(buffer))
    {
      return m_model;
    }
    else
    {
      return nullptr;
    }
  }

  std::shared_ptr<Daruma> model()
  {
    return m_model;
  }

private:
  void initModel()
  {
    auto build_design_doc_fn =
      [&, design_schema_file_path = m_designSchemaFilePath](const json& designJson)
    {
      auto json_doc_ptr = createJsonDoc();
      json_doc_ptr->setContent(designJson);

      if (!design_schema_file_path.empty())
      {
        SchemaValidJsonDocument::ValidatorPtr design_doc_validator;
        std::ifstream schemaIfs(design_schema_file_path);
        json schema = json::parse(schemaIfs);
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
    return new RawJsonDocument();
    // todo, use automerge
    return new UndoRedoJsonDocument();
  }
};

} // namespace VGG
