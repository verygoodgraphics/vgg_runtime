#include "UIView.hpp"

#include <SDL.h>

using namespace VGG;

void UIView::onEvent(const SDL_Event& evt)
{
  switch (evt.type)
  {
    case SDL_MOUSEBUTTONDOWN:
    {
      if (m_event_listener)
      {
        // todo, hittest
        auto p = UIEventPtr(new MouseEvent("/fake/update_background_color", UIEventType::click));
        m_event_listener(p);
      }
    }
    break;

      // todo, more event

    default:
      break;
  }
}