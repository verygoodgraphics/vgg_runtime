#ifndef VGG_JSON_SCHEMA_VALIDATOR_HPP
#define VGG_JSON_SCHEMA_VALIDATOR_HPP

#include "nlohmann/json.hpp"
#include "valijson_nlohmann_bundled.hpp"

#include <map>

class JsonSchemaValidator
{
public:
  JsonSchemaValidator();
  ~JsonSchemaValidator();

  void setRootSchema(const nlohmann::json& schemaJson);
  bool validate(const nlohmann::json& targetDocument);
  bool validate(const std::string& className, const nlohmann::json& targetDocument);

private:
  valijson::Validator validator_;
  valijson::Schema schema_;
  std::map<std::string, std::string> classTitleMap_;
  std::map<std::string, const valijson::Subschema*> classSubschemaMap_;

  void preProcessSchemaAndSetupMap(nlohmann::json& schemaJson);
  void setRootSchemaInternal(const nlohmann::json& schemaJson);
  bool validate(const valijson::Subschema* subschema, const nlohmann::json& targetDocument);
  const valijson::Subschema* getSubschemaByClassName(const std::string& className);
};

#endif