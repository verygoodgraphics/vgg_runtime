#pragma once
#include "Event/EventListener.hpp"

#include "VGG/Layer/Renderable.hpp"

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
