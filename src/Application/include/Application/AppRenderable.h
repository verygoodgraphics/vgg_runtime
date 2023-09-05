#pragma once

#include "Scene/Renderable.h"
#include "Application/interface/Event/EventListener.h"

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
