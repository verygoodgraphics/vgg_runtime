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
#include "JsonSchemaValidator.hpp"

#include "Utility/Log.hpp"

using nlohmann::json;

JsonSchemaValidator::JsonSchemaValidator()
{
}

JsonSchemaValidator::~JsonSchemaValidator()
{
}

void JsonSchemaValidator::setRootSchema(nlohmann::json& schemaJson)
{
  preProcessSchemaAndSetupMap(schemaJson);
  setRootSchemaInternal(schemaJson);
}

bool JsonSchemaValidator::validate(const nlohmann::json& targetDocument)
{
  return validate(&m_schema, targetDocument);
}

bool JsonSchemaValidator::validate(const std::string& className,
                                   const nlohmann::json& targetDocument)
{
  return validate(getSubschemaByClassName(className), targetDocument);
}

bool JsonSchemaValidator::validate(const valijson::Subschema* subschema,
                                   const nlohmann::json& targetDocument)
{
  if (!subschema)
  {
    return false;
  }

  valijson::ValidationResults results;
  valijson::adapters::NlohmannJsonAdapter targetDocumentAdapter(targetDocument);
  if (!m_validator.validate(*subschema, targetDocumentAdapter, &results))
  {
    valijson::ValidationResults::Error error;
    unsigned int errorNum = 1;
    while (results.popError(error))
    {
      std::string context;
      std::vector<std::string>::iterator itr = error.context.begin();
      for (; itr != error.context.end(); itr++)
      {
        context += *itr;
      }

      WARN("#vgg json schema validate error: %d, context: %s, desc: %s",
           errorNum,
           context.c_str(),
           error.description.c_str());
      ++errorNum;
    }
    return false;
  }
  return true;
}

const valijson::Subschema* JsonSchemaValidator::getSubschemaByClassName(
  const std::string& className)
{
  try
  {
    auto result = m_classSubschemaMap.at(className);
    return result;
  }
  catch (std::out_of_range&)
  {
    try
    {
      auto& title = m_classTitleMap.at(className);
      auto item = m_schema.getSubschemaByTitle(title);
      if (item)
      {
        m_classSubschemaMap[className] = item;
      }
      return item;
    }
    catch (std::out_of_range&)
    {
      return nullptr;
    }
  }
  catch (...)
  {
    return nullptr;
  }
}

void JsonSchemaValidator::preProcessSchemaAndSetupMap(nlohmann::json& schemaJson)
{
  auto& definition_object = schemaJson["definitions"];

  m_classTitleMap.clear();
  m_classSubschemaMap.clear();
  for (auto it = definition_object.begin(); it != definition_object.end(); ++it)
  {
    if (it.value().is_object())
    {
      auto& class_name_json = it.value()["properties"]["class"]["const"];
      if (class_name_json.is_string())
      {
        // make title unique
        auto new_title = it.value()["$id"].get<std::string>();
        it.value()["title"] = new_title;

        // save to class:title map
        auto className = class_name_json.get<std::string>();
        m_classTitleMap[className] = new_title;
      }
    }
  }
}

void JsonSchemaValidator::setRootSchemaInternal(const nlohmann::json& schemaJson)
{
  valijson::SchemaParser parser;
  valijson::adapters::NlohmannJsonAdapter schemaDocumentAdapter(schemaJson);
  try
  {
    parser.populateSchema(schemaDocumentAdapter, m_schema);
  }
  catch (std::exception& e)
  {
    WARN("#vgg json schema set schema error: %s", e.what());
    throw;
  }
}
