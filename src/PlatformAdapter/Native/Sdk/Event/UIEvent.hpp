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

template<class T>
class UIEvent : public Event<T>
{
};

} // namespace NodeAdapter
} // namespace VGG