#pragma once

#include <any>
#include <memory>
#include <string>

namespace VGG
{
enum class ViewEventType
{
  Invalid,
  Click
};

struct ViewEvent
{
  ViewEventType type;
  std::any data;
};
using ViewEventPtr = std::shared_ptr<ViewEvent>;

struct ViewEventClick
{
  const std::string path;
};

} // namespace VGG