#pragma once

#include "nlohmann/json.hpp"

#include <any>
#include <memory>
#include <utility>

namespace VGG
{
enum class ModelEventType
{
  Invalid,
  Add,
  Delete,
  Update
};

struct ModelEvent
{
  using PathType = nlohmann::json::json_pointer;
  using ValueType = nlohmann::json;

  const ModelEventType type;
  const PathType path;

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