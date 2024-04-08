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
#include "SubjectJsonDocument.hpp"

#include <algorithm>

using namespace VGG;

void SubjectJsonDocument::addAt(const json::json_pointer& path, const json& value)
{
  JsonDocument::addAt(path, value);

  m_subject.get_subscriber().on_next(ModelEventPtr{ new ModelEventAdd{ path, value } });
}

void SubjectJsonDocument::replaceAt(const json::json_pointer& path, const json& value)
{
  JsonDocument::replaceAt(path, value);

  m_subject.get_subscriber().on_next(ModelEventPtr{ new ModelEventUpdate{ path, value } });
}

void SubjectJsonDocument::deleteAt(const json::json_pointer& path)
{
  JsonDocument::deleteAt(path);

  m_subject.get_subscriber().on_next(ModelEventPtr{ new ModelEventDelete{ path } });
}

void SubjectJsonDocument::updateElement(const std::string& id, const std::string& contentJsonString)
{
  JsonDocument::updateElement(id, contentJsonString);
  m_subject.get_subscriber().on_next(
    ModelEventPtr{ new ModelEventUpdate{ id, contentJsonString } });
}
