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

#include "ModelEvent.hpp"

#include <rxcpp/rx.hpp>

#include <memory>
#include <vector>

class SubjectJsonDocument : public JsonDocument
{
  rxcpp::subjects::subject<VGG::ModelEventPtr> m_subject;

public:
  SubjectJsonDocument(JsonDocumentPtr jsonDoc)
    : JsonDocument(jsonDoc)
  {
  }

  auto getObservable()
  {
    return m_subject.get_observable();
  }

  virtual void addAt(const json::json_pointer& path, const json& value) override;
  virtual void replaceAt(const json::json_pointer& path, const json& value) override;
  virtual void deleteAt(const json::json_pointer& path) override;
};
