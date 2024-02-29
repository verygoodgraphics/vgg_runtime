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
#include "Layer/PathGenerator.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Utility/Log.hpp"
#include <core/SkClipOp.h>
#include <core/SkPaint.h>
#include <core/SkPath.h>
#include <core/SkPathTypes.h>
#include <core/SkRRect.h>
#include <core/SkCanvas.h>

#include <glm/glm.hpp>
#include <vector>

#include <optional>
#include <variant>
namespace VGG::layer
{

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

struct Ellipse
{
  SkRect rect;
  Ellipse() = default;
  Ellipse(Ellipse&& oval) noexcept = default;
  Ellipse& operator=(Ellipse&& oval) noexcept = default;
  Ellipse(const Ellipse& oval) = default;
  Ellipse& operator=(const Ellipse& oval) = default;
  Ellipse(const SkRect& rect)
    : rect(rect)
  {
  }
};

class Shape : public std::enable_shared_from_this<Shape>
{
public:
  virtual void                  draw(SkCanvas* canvas, const SkPaint& paint) const = 0;
  virtual void                  clip(SkCanvas* canvas, SkClipOp clipOp) const = 0;
  virtual SkPath                asPath() = 0;
  virtual SkRect                bound() = 0;
  virtual std::optional<SkRect> visualBound()
  {
    return bound();
  }

  bool isClosed() const
  {
    return m_closed;
  }
  bool isEmpty() const
  {
    return m_empty;
  }

  virtual ~Shape() = default;

protected:
  void setEmpty(bool empty)
  {
    m_empty = empty;
  }
  void setClosed(bool closed)
  {
    m_closed = closed;
  }
  bool m_closed{ false };
  bool m_empty{ true };
};

class ArcShape final : public Shape
{
public:
  ArcShape(const SkRect& oval, float startAngle, float sweepAngle, bool useCenter)
  {
    m_oval = oval;
    m_startAngle = startAngle;
    m_sweepAngle = sweepAngle;
    m_useCenter = useCenter;
    setEmpty(false);
    setClosed(true);
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawArc(m_oval, m_startAngle, m_sweepAngle, m_useCenter, paint);
  }
  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipRect(m_oval, clipOp);
  }
  // void transform(const SkMatrix& matrix) override
  // {
  //   m_oval = matrix.mapRect(m_oval);
  // }
  SkPath asPath() override
  {
    SkPath path;
    path.addOval(m_oval);
    return path;
  }

  SkRect bound() override
  {
    return m_oval;
  }
  std::optional<SkRect> visualBound() override
  {
    return m_oval;
  }
  SkRect ellipse() const
  {
    return m_oval;
  }

private:
  SkRect m_oval;
  float  m_startAngle;
  float  m_sweepAngle;
  float  m_useCenter;
};

struct ShapePath
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

  ShapePath()
    : m_type(EMPTY)
  {
  }

  ~ShapePath();

  ShapePath(const ShapePath& shape)
  {
    *this = shape;
  }

  ShapePath& operator=(const ShapePath& shape)
  {
    m_type = shape.m_type;
    m_impl = shape.m_impl;
    return *this;
  }

  explicit ShapePath(const SkPath& path)
  {
    setPath(path);
  }

  explicit ShapePath(ContourPtr contour)
  {
    setContour(contour);
  }

  explicit ShapePath(const SkRect& rect)
  {
    setRect(rect);
  }

  explicit ShapePath(const SkRRect& rrect)
  {
    setRRect(rrect);
  }

  explicit ShapePath(const Ellipse& oval)
  {
    setOval(oval);
  }

  explicit ShapePath(const Arc& arc)
  {
    setArc(arc.oval, arc.startAngle, arc.sweepAngle, arc.useCenter);
  }

  void setPath(const SkPath& path);

  void setContour(const ContourPtr& contour);

  void setRect(const SkRect& rect);

  std::optional<SkRect> asRect() const;

  void setRRect(const SkRRect& rrect);

  std::optional<SkRRect> asRRect() const;

  void setArc(const SkRect& oval, float startAngle, float sweepAngle, bool forceMoveTo);
  void setOval(const Ellipse& ellipse);

  std::optional<Ellipse> asOval() const;

  void op(const ShapePath& shape, EBoolOp op);

  void transform(ShapePath& shape, const SkMatrix& matrix);

  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    m_impl->clip(canvas, clipOp);
  }

  void setFillType(EWindingType fillType);

  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    m_impl->draw(canvas, paint);
  }

  std::optional<ShapePath> outset(float x, float y) const;

  SkPath asPath() const
  {
    if (m_impl == nullptr)
      return SkPath();
    return m_impl->asPath();
  }

  bool isEmpty() const;
  bool isClosed() const
  {
    return m_impl->isClosed();
  }

  uint8_t type() const
  {
    return m_type;
  }

  void reset()
  {
    m_type = EMPTY;
    m_impl.reset();
  }

private:
  std::shared_ptr<Shape> m_impl;
  uint8_t                m_type{ EMPTY };
  SkMatrix               m_transform;
}; // namespace VGG::layer
} // namespace VGG::layer
