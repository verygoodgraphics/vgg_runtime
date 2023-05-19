#pragma once

#include "nlohmann/json.hpp"

#include <any>
#include <memory>

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
  ModelEventType type;
  std::any data;
};
using ModelEventPtr = std::shared_ptr<ModelEvent>;

struct ModelEventAdd
{
  const nlohmann::json::json_pointer path;
  const nlohmann::json value;
};

struct ModelEventDelete
{
  const nlohmann::json::json_pointer path;
};

struct ModelEventUpdate
{
  const nlohmann::json::json_pointer path;
  const nlohmann::json value;
};

} // namespace VGG