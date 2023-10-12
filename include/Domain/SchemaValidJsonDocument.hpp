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
#pragma once

#include "JsonDocument.hpp"
#include "JsonSchemaValidator.hpp"

#include <memory>

class SchemaValidJsonDocument : public JsonDocument
{
public:
  using ValidatorPtr = std::shared_ptr<JsonSchemaValidator>;
  SchemaValidJsonDocument(const JsonDocumentPtr& jsonDoc, const ValidatorPtr& schemaValidator);

  void addAt(const json::json_pointer& path, const json& value) override;
  void replaceAt(const json::json_pointer& path, const json& value) override;
  void deleteAt(const json::json_pointer& path) override;

private:
  const ValidatorPtr m_validator;

  void editTemplate(
    const json::json_pointer& path,
    const json& value,
    std::function<void(json&, json::json_pointer&, const json&)> tryEditFn,
    std::function<void(JsonDocumentPtr&, const json::json_pointer&, const json&)> editFn);
  bool validateDocument(const json& document);
  const json::json_pointer getNearestHavingClassAncestorPath(
    const json::json_pointer& editPath) const;
  void calculateRelativePath(const json::json_pointer& ancestorPath,
                             const json::json_pointer& currentPath,
                             json::json_pointer& relativePath);
};
