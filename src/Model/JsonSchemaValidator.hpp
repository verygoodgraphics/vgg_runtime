#ifndef VGG_JSON_SCHEMA_VALIDATOR_HPP
#define VGG_JSON_SCHEMA_VALIDATOR_HPP

#include "nlohmann/json.hpp"
#include "nlohmann/json-schema.hpp"

class JsonSchemaValidator
{
public:
  JsonSchemaValidator();
  ~JsonSchemaValidator();

  void setRootSchema(const nlohmann::json& schema);
  void setRootSchema(nlohmann::json&& schema);

  bool validate(const nlohmann::json&) const;

private:
  nlohmann::json_schema::json_validator validator_;
};

#endif