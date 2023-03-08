#include "JsonSchemaValidator.hpp"

#include "Utils/Utils.hpp"

using namespace valijson;
using namespace valijson::adapters;
using nlohmann::json;

JsonSchemaValidator::JsonSchemaValidator()
{
}

JsonSchemaValidator::~JsonSchemaValidator()
{
}

void JsonSchemaValidator::setRootSchema(const nlohmann::json& schemaJson)
{
  auto tmp_json = schemaJson;
  preProcessSchemaAndSetupMap(tmp_json);
  setRootSchemaInternal(tmp_json);
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

  ValidationResults results;
  NlohmannJsonAdapter targetDocumentAdapter(targetDocument);
  if (!m_validator.validate(*subschema, targetDocumentAdapter, &results))
  {
    ValidationResults::Error error;
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
  catch (std::out_of_range)
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
    catch (std::out_of_range)
    {
      return nullptr;
    }
  }
}

void JsonSchemaValidator::preProcessSchemaAndSetupMap(nlohmann::json& schemaJson)
{
  auto& defination_object = schemaJson["definitions"];

  m_classTitleMap.clear();
  m_classSubschemaMap.clear();
  for (auto it = defination_object.begin(); it != defination_object.end(); ++it)
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
        auto class_name = class_name_json.get<std::string>();
        m_classTitleMap[class_name] = new_title;
      }
    }
  }
}

void JsonSchemaValidator::setRootSchemaInternal(const nlohmann::json& schemaJson)
{
  SchemaParser parser;
  NlohmannJsonAdapter schemaDocumentAdapter(schemaJson);
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
