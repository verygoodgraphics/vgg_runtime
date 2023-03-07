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
  processSchemaJsonAndSetupMap(tmp_json);
  setRootSchemaInternal(tmp_json);
}

bool JsonSchemaValidator::validate(const nlohmann::json& targetDocument)
{
  return validate(&schema_, targetDocument);
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
  if (!validator_.validate(*subschema, targetDocumentAdapter, &results))
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

valijson::Subschema* JsonSchemaValidator::getSubschemaByClassName(const std::string& className)
{
  return nullptr;
}

void JsonSchemaValidator::processSchemaJsonAndSetupMap(nlohmann::json& schemaJson)
{
  auto& defination_object = schemaJson["definitions"];

  classTitleMap_.clear();
  for (auto it = defination_object.begin(); it != defination_object.end(); ++it)
  {
    if (it.value().is_object())
    {
      auto& class_name = it.value()["properties"]["class"]["const"];
      if (class_name.is_string())
      {
        // crash when pupulate schema
        // // fill id field to scheme json
        // auto id_str = it.value()["$id"].dump();
        // it.value()["id"] = id_str;

        // save to class:id map
        auto title = it.value()["title"].dump();
        auto class_name_str = class_name.dump();
        classTitleMap_[class_name_str] = title;
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
    parser.populateSchema(schemaDocumentAdapter, schema_);
  }
  catch (std::exception& e)
  {
    WARN("#vgg json schema set schema error: %s", e.what());
    throw;
  }
}
