#ifndef VGG_JSON_SCHEMA_VALIDATOR_HPP
#define VGG_JSON_SCHEMA_VALIDATOR_HPP

#include "nlohmann/json.hpp"
#include "valijson_nlohmann_bundled.hpp"

#include <unordered_map>

class JsonSchemaValidator
{
public:
  JsonSchemaValidator();
  ~JsonSchemaValidator();

  void setRootSchema(nlohmann::json& schemaJson);
  bool validate(const nlohmann::json& targetDocument);
  bool validate(const std::string& className, const nlohmann::json& targetDocument);

private:
  valijson::Validator m_validator;
  valijson::Schema m_schema;
  std::unordered_map<std::string, std::string> m_classTitleMap;
  std::unordered_map<std::string, const valijson::Subschema*> m_classSubschemaMap;

  void preProcessSchemaAndSetupMap(nlohmann::json& schemaJson);
  void setRootSchemaInternal(const nlohmann::json& schemaJson);
  bool validate(const valijson::Subschema* subschema, const nlohmann::json& targetDocument);
  const valijson::Subschema* getSubschemaByClassName(const std::string& className);
};

#endif