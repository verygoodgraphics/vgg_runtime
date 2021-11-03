/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __INPUT_MANAGER_HPP__
#define __INPUT_MANAGER_HPP__

#include <SDL2/SDL.h>

#include "Entity/MouseEntity.hpp"
#include "Entity/KeyboardEntity.hpp"
#include "Systems/RenderSystem.hpp"
#include "Utils/Utils.hpp"

namespace VGG
{

class InputManager
{
public:
  using MouseMove = MouseEntity::MouseMove;
  using MouseClick = MouseEntity::MouseClick;
  using MouseRelease = MouseEntity::MouseRelease;
  using KeyboardPress = KeyboardEntity::KeyboardPress;
  using KeyboardRelease = KeyboardEntity::KeyboardRelease;
  using KeyboardText = KeyboardEntity::KeyboardText;

  struct InputEvent
  {
  };

  struct EachFrame : InputEvent
  {
  };

private:
  MouseEntity mouse;
  KeyboardEntity keyboard;
  bool cursorVisible{ true };

  static InputManager* getInstance()
  {
    static InputManager im;
    return &im;
  }

public:
  static void setCursorVisibility(bool visible)
  {
    if (auto im = getInstance())
    {
      im->cursorVisible = visible;
    }
  }

  static void setMouseCursor(MouseEntity::CursorType ct)
  {
    auto im = getInstance();
    ASSERT(im);
    im->mouse.setCursor(ct);
  }

  static Vec2 getGlobalMousePos()
  {
    auto im = getInstance();
    ASSERT(im);
    return Vec2{ im->mouse.x, im->mouse.y };
  }

  static void draw(SkCanvas* canvas)
  {
    auto im = getInstance();
    ASSERT(im);
    ASSERT(canvas);

    if (im->cursorVisible)
    {
      RenderSystem::drawEntity(canvas, im->mouse);
    }
  }

  static void onEvent(const SDL_Event& evt)
  {
    auto im = getInstance();
    ASSERT(im);

    if (evt.type == SDL_MOUSEMOTION)
    {
      im->mouse.x = SINT2FLOAT(evt.motion.x);
      im->mouse.y = SINT2FLOAT(evt.motion.y);
      im->mouse.publish<MouseMove>(MouseMove{
        .modKey = SDL_GetModState(),
        .dx = SINT2FLOAT(evt.motion.xrel),
        .dy = SINT2FLOAT(evt.motion.yrel),
      });
    }
    else if (evt.type == SDL_MOUSEBUTTONDOWN)
    {
      im->mouse.isDragging = true;
      im->mouse.publish<MouseClick>(MouseClick{
        .modKey = SDL_GetModState(),
        .nClicks = evt.button.clicks,
        .button = evt.button.button,
      });
    }
    else if (evt.type == SDL_MOUSEBUTTONUP)
    {
      im->mouse.isDragging = false;
      im->mouse.publish<MouseRelease>(MouseRelease{
        .button = evt.button.button,
      });
    }
    else if (evt.type == SDL_KEYDOWN)
    {
      im->keyboard.publish<KeyboardPress>(KeyboardPress{
        .modKey = SDL_GetModState(),
        .key = evt.key.keysym.sym,
      });
    }
    else if (evt.type == SDL_KEYUP)
    {
      im->keyboard.publish<KeyboardRelease>(KeyboardRelease{
        .modKey = SDL_GetModState(),
        .key = evt.key.keysym.sym,
      });
    }
    else if (evt.type == SDL_TEXTINPUT)
    {
      im->keyboard.publish<KeyboardText>(KeyboardText{
        .text = evt.text.text,
      });
    }
  }

  static void onFrame()
  {
    EntityManager::accept<EachFrame>(EachFrame{});
  }
};

}; // namespace VGG

#endif // __INPUT_MANAGER_HPP__
