#include "JsonSchemaValidator.hpp"

#include "Utils/Utils.hpp"

JsonSchemaValidator::JsonSchemaValidator()
{
}

JsonSchemaValidator::~JsonSchemaValidator()
{
}

void JsonSchemaValidator::setRootSchema(const nlohmann::json& schema)
{
  validator_.set_root_schema(schema);
}

void JsonSchemaValidator::setRootSchema(nlohmann::json&& schema)
{
  validator_.set_root_schema(schema);
}

bool JsonSchemaValidator::validate(const nlohmann::json& json_object) const
{
  try
  {
    validator_.validate(json_object);
    return true;
  }
  catch (const std::exception& e)
  {
    WARN("#vgg json schema validate error: %s", e.what());
    return false;
  }
}