#pragma once
#include "Event/EventListener.h"

#include "VGG/Layer/Renderable.h"

namespace VGG::app
{
class AppRenderable
  : public layer::Renderable
  , public EventListener
{
public:
  AppRenderable() = default;
};
} // namespace VGG::app
