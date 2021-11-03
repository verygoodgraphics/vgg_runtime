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
#ifndef __MOUSE_ENTITY_HPP__
#define __MOUSE_ENTITY_HPP__

#include "Entity/EntityManager.hpp"
#include "Presets/Paths/MouseCursorPaths.hpp"
#include "Presets/Styles/DefaultPathStyle.hpp"

namespace VGG
{

struct MouseEntity : Entity
{
  constexpr static double CURSOR_SIZE = 24.0;

  struct MouseEvent
  {
    MouseEntity* source{ nullptr };
  };

  struct MouseMove : MouseEvent
  {
    size_t modKey{ 0 };
    double dx{ 0. };
    double dy{ 0. };
  };

  struct MouseClick : MouseEvent
  {
    size_t modKey{ 0 };
    size_t nClicks{ 0 };
    size_t button{ 0 };
  };

  struct MouseRelease : MouseEvent
  {
    size_t button{ 0 };
  };

  enum class CursorType
  {
    NORMAL,
    RESIZE_HORZ,
    RESIZE_VERT,
    RESIZE_DIAG,
    RESIZE_ANTI_DIAG,
    MOVE,
    TEXT,
    NEW_NODE,
  };

  struct SelectionPoint
  {
    double x{ 0. };
    double y{ 0. };
  };

public:
  CursorType cursor{ CursorType::NORMAL };
  double x{ 0. }; // in global space
  double y{ 0. }; // in global space
  bool isDragging{ false };
  std::optional<SelectionPoint> selectionPoint{ std::nullopt };
  std::optional<Entity> previewEntity{ std::nullopt };
  std::vector<Vec2> trajectory;

  inline Frame getSelectionFrame() const
  {
    if (!selectionPoint.has_value())
    {
      WARN("No selection point.");
    }
    auto spv = selectionPoint.value_or(SelectionPoint{});
    double sx = spv.x;
    double sy = spv.y;
    double x = std::min(sx, this->x);
    double y = std::min(sy, this->y);
    double w = std::fabs(sx - this->x);
    double h = std::fabs(sy - this->y);
    return Frame{ x, y, w, h };
  }

public:
  MouseEntity()
  {
    setCursor(CursorType::NORMAL);
  }

  inline void setCursor(CursorType ct)
  {
    cursor = ct;

    auto style = DefaultPathStyle::createCursor();
    switch (ct)
    {
      case CursorType::NORMAL:
      {
        renderable<FramedPath>(FramedPath{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .path = MouseCursorPaths::createPointer(),
          .style = style,
        });
        break;
      }
      case CursorType::RESIZE_HORZ:
      {
        renderable<FramedRelation>(FramedRelation{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .children = {
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { -CURSOR_SIZE/2, -CURSOR_SIZE/2, CURSOR_SIZE, CURSOR_SIZE },
              .path = MouseCursorPaths::createHorzBiArrow(),
              .style = style,
            }),
          },
        });
        break;
      }
      case CursorType::RESIZE_VERT:
      {
        renderable<FramedRelation>(FramedRelation{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .children = {
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { -CURSOR_SIZE/2, -CURSOR_SIZE/2, CURSOR_SIZE, CURSOR_SIZE },
              .path = MouseCursorPaths::createVertBiArrow(),
              .style = style,
            }),
          },
        });
        break;
      }
      case CursorType::RESIZE_DIAG:
      {
        renderable<FramedRelation>(FramedRelation{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .children = {
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { -CURSOR_SIZE/2, -CURSOR_SIZE/2, CURSOR_SIZE, CURSOR_SIZE },
              .path = MouseCursorPaths::createDiagBiArrow(),
              .style = style,
            }),
          },
        });
        break;
      }
      case CursorType::RESIZE_ANTI_DIAG:
      {
        renderable<FramedRelation>(FramedRelation{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .children = {
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { -CURSOR_SIZE/2, -CURSOR_SIZE/2, CURSOR_SIZE, CURSOR_SIZE },
              .path = MouseCursorPaths::createAntiDiagBiArrow(),
              .style = style,
            }),
          },
        });
        break;
      }
      case CursorType::MOVE:
      {
        renderable<FramedRelation>(FramedRelation{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .children = {
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
              .path = MouseCursorPaths::createPointer(),
              .style = style,
            }),
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { 3*CURSOR_SIZE/5, 4*CURSOR_SIZE/5, CURSOR_SIZE/2, CURSOR_SIZE/2 },
              .path = MouseCursorPaths::createCrossBiArrows(),
              .style = style,
            }),
          },
        });
        break;
      }
      case CursorType::TEXT:
      {
        renderable<FramedRelation>(FramedRelation{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .children = {
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { -CURSOR_SIZE/2, -CURSOR_SIZE/2, CURSOR_SIZE, CURSOR_SIZE },
              .path = MouseCursorPaths::createTextInput(),
              .style = style,
            }),
          },
        });
        break;
      }
      case CursorType::NEW_NODE:
      {
        renderable<FramedRelation>(FramedRelation{
          .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
          .children = {
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { 0, 0, CURSOR_SIZE, CURSOR_SIZE },
              .path = MouseCursorPaths::createPointer(),
              .style = style,
            }),
            Entity{}.renderable<FramedPath>(FramedPath{
              .frame = { 3*CURSOR_SIZE/5, 4*CURSOR_SIZE/5, CURSOR_SIZE/2, CURSOR_SIZE/2 },
              .path = MouseCursorPaths::createCrossLines(),
              .style = style,
            }),
          },
        });
        break;
      }
    }
  }

  template<typename EventType>
  void publish(EventType evt)
  {
    if constexpr (std::is_base_of_v<MouseEvent, EventType>)
    {
      evt.source = this;
    }
    EntityManager::accept<EventType>(evt);
  }
};

}; // namespace VGG

#endif // __MOUSE_ENTITY_HPP__
