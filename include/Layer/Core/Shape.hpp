/*
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
#include "Layer/Core/VType.hpp"
#include "Utility/Log.hpp"
#include <core/SkClipOp.h>
#include <core/SkPaint.h>
#include <core/SkPath.h>
#include <core/SkRRect.h>
#include <core/SkCanvas.h>

#include <glm/glm.hpp>
#include <vector>

#include <optional>
namespace VGG::layer
{

struct Shape
{
public:
  enum EType : uint8_t
  {
    EMPTY,
    PATH,
    RECT,
    RRECT,
    ARCH,
    OVAL
  };

  struct Arc
  {
    SkRect oval;
    float  startAngle;
    float  sweepAngle;
    bool   useCenter;
    Arc() = default;
    Arc(Arc&& arc) noexcept = default;
    Arc& operator=(Arc&& arc) noexcept = default;
    Arc(const Arc& arc) = default;
    Arc& operator=(const Arc& arc) = default;
    Arc(const SkRect& oval, float startAngle, float sweepAngle, bool useCenter)
      : oval(oval)
      , startAngle(startAngle)
      , sweepAngle(sweepAngle)
      , useCenter(useCenter)
    {
    }
  };

  struct Oval
  {
    SkRect rect;
    Oval() = default;
    Oval(Oval&& oval) noexcept = default;
    Oval& operator=(Oval&& oval) noexcept = default;
    Oval(const Oval& oval) = default;
    Oval& operator=(const Oval& oval) = default;
    Oval(const SkRect& rect)
      : rect(rect)
    {
    }
  };

  Shape()
    : m_type(EMPTY)
  {
  }

  ~Shape()
  {
    if (type() == PATH)
    {
      m_u.path.~SkPath();
    }
  }

  Shape(const Shape& shape)
  {
    *this = shape;
  }

  Shape& operator=(const Shape& shape)
  {
    switch (shape.type())
    {
      case PATH:
        setPath(shape.m_u.path);
        break;
      case RECT:
        setRect(shape.m_u.rect);
        break;
      case RRECT:
        setRRect(shape.m_u.rrect);
        break;
      case ARCH:
        setArch(
          shape.m_u.arc.oval,
          shape.m_u.arc.startAngle,
          shape.m_u.arc.sweepAngle,
          shape.m_u.arc.useCenter);
        break;
      case OVAL:
        setOval(shape.m_u.oval);
      case EMPTY:
        reset();
        break;
    }
    return *this;
  }

  bool isEmpty() const
  {
    return m_type == EMPTY || (m_type == PATH && m_u.path.isEmpty());
  }

  explicit Shape(const SkPath& path)
  {
    setPath(path);
  }

  explicit Shape(const SkRect& rect)
  {
    setRect(rect);
  }

  explicit Shape(const SkRRect& rrect)
  {
    setRRect(rrect);
  }

  explicit Shape(const Oval& oval)
  {
    setOval(oval);
  }

  explicit Shape(const Arc& arc)
  {
    setArch(arc.oval, arc.startAngle, arc.sweepAngle, arc.useCenter);
  }

  void setPath(const SkPath& path)
  {
    if (this->type() == PATH)
    {
      m_u.path = path;
    }
    else
    {
      new (&m_u.path) SkPath(path);
      m_type = PATH;
    }
  }

  void setRect(const SkRect& rect)
  {
    this->m_u.rect = rect;
    m_type = RECT;
  }

  void setRRect(const SkRRect& rrect)
  {
    this->m_u.rrect = rrect;
    m_type = RRECT;
  }

  void setArch(const SkRect& oval, float startAngle, float sweepAngle, bool forceMoveTo)
  {
    this->m_u.arc.oval = oval;
    m_type = ARCH;
  }

  void setOval(const Oval& oval)
  {
    this->m_u.oval = oval;
    m_type = OVAL;
  }

  void op(const Shape& shape, EBoolOp op);

  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    switch (m_type)
    {
      case PATH:
        clipPath(canvas, m_u.path, clipOp);
        break;
      case RECT:
        clipRect(canvas, m_u.rect, clipOp);
        break;
      case RRECT:
        clipRRect(canvas, m_u.rrect, clipOp);
        break;
      case ARCH:
        clipArc(canvas, m_u.arc, clipOp);
        break;
    }
  }

  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    switch (this->type())
    {
      case PATH:
        drawPath(canvas, m_u.path, paint);
        break;
      case RECT:
        drawRect(canvas, m_u.rect, paint);
        break;
      case RRECT:
        drawRRect(canvas, m_u.rrect, paint);
        break;
      case ARCH:
        drawOval(canvas, m_u.oval, paint);
        break;
    }
  }

  SkPath asPath() const
  {
    switch (this->type())
    {
      case PATH:
        return m_u.path;
      case RECT:
        return SkPath::Rect(m_u.rect);
      case RRECT:
        return SkPath::RRect(m_u.rrect);
      case OVAL:
        return SkPath::Oval(m_u.oval.rect);
      case ARCH:
        return SkPath();
    }
    return SkPath();
  }

  bool isClosed() const
  {
    switch (this->type())
    {
      case PATH:
        return m_u.path.isLastContourClosed();
      case RECT:
        return true;
      case RRECT:
        return true;
      case OVAL:
        return true;
      case ARCH:
        return true;
    }
    return true;
  }

  uint8_t type() const
  {
    return m_type;
  }

  void reset()
  {
    m_type = EMPTY;
  }

private:
  union UData
  {
    SkPath  path;
    SkRect  rect;
    SkRRect rrect;
    Oval    oval;
    Arc     arc;
    UData()
    {
    }

    ~UData()
    {
    }
    UData(const UData& data)
    {
    }
    UData(UData&& data) noexcept
    {
    }

    UData& operator=(UData&& data) noexcept
    {
      return *this;
    }

    UData& operator=(const UData& data)
    {
      return *this;
    }
  } m_u;

  uint8_t m_type{ EMPTY };

  static void drawOval(SkCanvas* canvas, const Oval& oval, const SkPaint& paint)
  {
    canvas->drawOval(oval.rect, paint);
  }

  static void clipOval(SkCanvas* canvas, const Oval& oval, SkClipOp clipOp)
  {
    canvas->clipRect(oval.rect, clipOp);
  }

  static void drawArc(SkCanvas* canvas, const Arc& arc, const SkPaint& paint)
  {
    canvas->drawArc(arc.oval, arc.startAngle, arc.sweepAngle, arc.useCenter, paint);
  }

  static void clipArc(SkCanvas* canvas, const Arc& arc, SkClipOp clipOp)
  {
    canvas->clipRect(arc.oval, clipOp);
  }

  static void drawRect(SkCanvas* canvas, const SkRect& rect, const SkPaint& paint)
  {
    canvas->drawRect(rect, paint);
  }

  static void clipRect(SkCanvas* canvas, const SkRect& rect, SkClipOp clipOp)
  {
    canvas->clipRect(rect, clipOp);
  }

  static void drawRRect(SkCanvas* canvas, const SkRRect& rect, const SkPaint& paint)
  {
    canvas->drawRRect(rect, paint);
  }

  static void clipRRect(SkCanvas* canvas, const SkRRect& rect, SkClipOp clipOp)
  {
    canvas->clipRRect(rect, clipOp);
  }

  static void drawPath(SkCanvas* canvas, const SkPath& path, const SkPaint& paint)
  {
    canvas->drawPath(path, paint);
  }

  static void clipPath(SkCanvas* canvas, const SkPath& path, SkClipOp clipOp)
  {
    canvas->clipPath(path, clipOp);
  }
}; // namespace VGG::layer
} // namespace VGG::layer
