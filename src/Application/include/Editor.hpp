#pragma once

#include "AppScene.h"
#include "UIEvent.hpp"

class SkCanvas;
namespace VGG
{

class Editor : public AppScene
{
  bool m_enabled{ true };

public:
  void enable(bool enabled)
  {
    m_enabled = enabled;
  }

  void handleUIEvent(UIEventPtr event);

  void onRender(SkCanvas* canvas) override;
};

} // namespace VGG