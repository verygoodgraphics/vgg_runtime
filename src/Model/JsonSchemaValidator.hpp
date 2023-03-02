#ifndef VGG_JSON_SCHEMA_VALIDATOR_HPP
#define VGG_JSON_SCHEMA_VALIDATOR_HPP

#include "nlohmann/json.hpp"
#include "valijson_nlohmann_bundled.hpp"

class JsonSchemaValidator
{
public:
  JsonSchemaValidator();
  ~JsonSchemaValidator();

  void setRootSchema(const nlohmann::json& schemaJson);
  bool validate(const nlohmann::json& targetJson);

private:
  valijson::Validator validator_;
  valijson::Schema schema_;
};

#endif