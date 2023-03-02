#include "JsonSchemaValidator.hpp"

#include "Utils/Utils.hpp"

using namespace valijson;
using namespace valijson::adapters;

JsonSchemaValidator::JsonSchemaValidator()
{
}

JsonSchemaValidator::~JsonSchemaValidator()
{
}

void JsonSchemaValidator::setRootSchema(const nlohmann::json& schemaJson)
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

bool JsonSchemaValidator::validate(const nlohmann::json& targetDocument)
{
  ValidationResults results;
  NlohmannJsonAdapter targetDocumentAdapter(targetDocument);
  if (!validator_.validate(schema_, targetDocumentAdapter, &results))
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