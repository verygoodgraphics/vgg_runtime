/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include "Application/Mouse.hpp"

#include <SDL.h>

using namespace VGG;

class SdlMouse : public Mouse
{
private:
  ECursor m_type{ ECursor::ARROW };
  SDL_Cursor* m_cursor{ nullptr };

public:
  ~SdlMouse()
  {
    freeCursor();
  }

  void setCursor(ECursor type) override
  {
    if (type == m_type)
    {
      return;
    }
    m_type = type;

    auto newType = SDL_SYSTEM_CURSOR_ARROW;
    switch (type)
    {
      case ECursor::ARROW:
        newType = SDL_SYSTEM_CURSOR_ARROW;
        break;
      case ECursor::IBEAM:
        newType = SDL_SYSTEM_CURSOR_IBEAM;
        break;
      case ECursor::WAIT:
        newType = SDL_SYSTEM_CURSOR_WAIT;
        break;
      case ECursor::CROSSHAIR:
        newType = SDL_SYSTEM_CURSOR_CROSSHAIR;
        break;
      case ECursor::WAITARROW:
        newType = SDL_SYSTEM_CURSOR_WAITARROW;
        break;
      case ECursor::SIZENWSE:
        newType = SDL_SYSTEM_CURSOR_SIZENWSE;
        break;
      case ECursor::SIZENESW:
        newType = SDL_SYSTEM_CURSOR_SIZENESW;
        break;
      case ECursor::SIZEWE:
        newType = SDL_SYSTEM_CURSOR_SIZEWE;
        break;
      case ECursor::SIZENS:
        newType = SDL_SYSTEM_CURSOR_SIZENS;
        break;
      case ECursor::SIZEALL:
        newType = SDL_SYSTEM_CURSOR_SIZEALL;
        break;
      case ECursor::NO:
        newType = SDL_SYSTEM_CURSOR_NO;
        break;
      case ECursor::HAND:
        newType = SDL_SYSTEM_CURSOR_HAND;
        break;
      default:
        break;
    }

    freeCursor();
    m_cursor = SDL_CreateSystemCursor(newType);
    SDL_SetCursor(m_cursor);
  }

  virtual void resetCursor() override
  {
    setCursor(ECursor::ARROW);
  }

private:
  void freeCursor()
  {
    if (m_cursor)
    {
      SDL_FreeCursor(m_cursor);
      m_cursor = nullptr;
    }
  }
};
