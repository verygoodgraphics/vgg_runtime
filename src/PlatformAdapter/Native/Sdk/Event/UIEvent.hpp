#pragma once

#include "PlatformAdapter/Native/Sdk/defines.hpp"
#include "Presenter/UIEvent.hpp"
#include "Event.hpp"

#include "js_native_api.h"

#include <memory>

namespace VGG
{
namespace NodeAdapter
{

template<class EventAdapterType, class EventType>
class UIEvent : public Event<EventAdapterType, EventType>
{
  using base_type = Event<EventAdapterType, EventType>;

protected:
  static auto properties()
  {
    auto base_properties{ base_type::properties() };

    decltype(base_properties) v;
    v.insert(v.end(), base_properties.begin(), base_properties.end());

    return v;
  }
};

} // namespace NodeAdapter
} // namespace VGG