/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#ifndef VGG_JSON_SCHEMA_VALIDATOR_HPP
#define VGG_JSON_SCHEMA_VALIDATOR_HPP

#include <nlohmann/json.hpp>
#include <valijson_nlohmann_bundled.hpp>

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
