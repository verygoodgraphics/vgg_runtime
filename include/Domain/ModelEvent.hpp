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

#include <nlohmann/json.hpp>

#include <any>
#include <memory>
#include <utility>

namespace VGG
{
enum class ModelEventType
{
  Invalid,

  // document
  Add,
  Delete,
  Update,

  // event listener
  ListenerDidAdd,
  ListenerDidRemove
};

struct ModelEvent
{
  using PathType = std::string;
  using ValueType = std::string;

  const ModelEventType type;
  const PathType       path;

  ModelEvent(const ModelEventType& type, const PathType& path)
    : type{ type }
    , path{ path }
  {
  }

  ModelEvent(ModelEventType&& type, PathType&& path)
    : type{ type }
    , path{ std::move(path) }
  {
  }

  virtual ~ModelEvent() = default;
};

using ModelEventPtr = std::shared_ptr<ModelEvent>;

struct ModelEventAdd : ModelEvent
{
  const ValueType value;

  ModelEventAdd(const PathType& path, const ValueType& value)
    : ModelEvent(ModelEventType::Add, path)
    , value{ value }
  {
  }

  ModelEventAdd(PathType&& path, ValueType&& value)
    : ModelEvent(ModelEventType::Add, std::move(path))
    , value{ std::move(value) }
  {
  }
};

struct ModelEventDelete : ModelEvent
{
  ModelEventDelete(const PathType& path)
    : ModelEvent(ModelEventType::Delete, path)
  {
  }

  ModelEventDelete(PathType&& path)
    : ModelEvent(ModelEventType::Delete, std::move(path))
  {
  }
};

struct ModelEventUpdate : ModelEvent
{
  const ValueType value;

  ModelEventUpdate(const PathType& path, const ValueType& value)
    : ModelEvent(ModelEventType::Update, path)
    , value{ value }
  {
  }

  ModelEventUpdate(PathType&& path, ValueType&& value)
    : ModelEvent(ModelEventType::Update, std::move(path))
    , value{ std::move(value) }
  {
  }
};

} // namespace VGG
