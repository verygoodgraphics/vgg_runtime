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

class ShapeBase : public std::enable_shared_from_this<ShapeBase>
{
public:
  virtual void   draw(SkCanvas* canvas, const SkPaint& paint) const = 0;
  virtual void   clip(SkCanvas* canvas, SkClipOp clipOp) const = 0;
  virtual void   transform(const SkMatrix& matrix) = 0;
  virtual SkPath asPath() = 0;
  bool           isClosed() const
  {
    return m_closed;
  }
  bool isEmpty() const
  {
    return m_empty;
  }

  virtual ~ShapeBase() = default;

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

class ContourShape : public ShapeBase
{
public:
  ContourShape(const ContourPtr& contour)
  {
    m_contour = contour;
  }

  ContourShape(const SkPath& path)
  {
    m_contour = nullptr;
    m_path = path;
    setEmpty(m_path->isEmpty());
    setClosed(m_path->isLastContourClosed());
  }

  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    ensurePath();
    if (m_path)
    {
      canvas->drawPath(*m_path, paint);
      DEBUG("Drawing path");
    }
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    ensurePath();
    if (m_path)
      canvas->clipPath(*m_path, clipOp);
  }
  void transform(const SkMatrix& matrix) override
  {
    ensurePath();
    if (m_path)
      m_path->transform(matrix);
  }

  SkPath asPath() override
  {
    ensurePath();
    if (m_path)
      return *m_path;
    return SkPath();
  }

  void setFillType(EWindingType fillType)
  {
    ensurePath();
    if (m_path)
      m_path->setFillType(
        EWindingType::WR_EVEN_ODD ? SkPathFillType::kEvenOdd : SkPathFillType::kWinding);
  }

private:
  // struct InternalPath
  // {
  // public:
  //   SkPath asPath()
  //   {
  //     return std::visit(
  //       [this](auto&& arg) -> SkPath
  //       {
  //         using T = std::decay_t<decltype(arg)>;
  //         if constexpr (std::is_same_v<T, SkPath>)
  //           return arg;
  //         else
  //         {
  //           SkPath p = VGG::layer::makePath(*arg);
  //           m_path = p;
  //           return p;
  //         }
  //       },
  //       m_path);
  //   }
  //   bool isClosed() const
  //   {
  //     return std::visit(
  //       [](auto&& arg) -> bool
  //       {
  //         using T = std::decay_t<decltype(arg)>;
  //         if constexpr (std::is_same_v<T, SkPath>)
  //           return arg.isLastContourClosed();
  //         else
  //         {
  //           return arg->closed;
  //         }
  //       },
  //       m_path);
  //   }
  //   void setPath(const SkPath& path)
  //   {
  //     m_path = path;
  //   }
  //   void setContour(const ContourPtr& contour)
  //   {
  //     m_path = contour;
  //   }
  //
  // private:
  //   using Path = std::variant<ContourPtr, SkPath>;
  //   Path m_path;
  //   void ensurePath()
  //   {
  //     std::visit(
  //       [this](auto&& arg) -> SkPath
  //       {
  //         using T = std::decay_t<decltype(arg)>;
  //         if constexpr (std::is_same_v<T, ContourPtr>)
  //         {
  //           SkPath p = VGG::layer::makePath(*arg);
  //           m_path = p;
  //         }
  //       },
  //       m_path);
  //   }
  // };
  ContourPtr            m_contour;
  std::optional<SkPath> m_path;
  void                  ensurePath() const
  {
    const_cast<ContourShape*>(this)->ensurePath();
  }

  void ensurePath()
  {
    if (!m_path && m_contour)
    {
      m_path = VGG::layer::makePath(*m_contour);
      setEmpty(m_path->isEmpty());
      setClosed(m_path->isLastContourClosed());
    }
  }
};

class RRectShape : public ShapeBase
{
public:
  RRectShape(const SkRRect& rect)
  {
    m_rect = rect;
    setEmpty(false);
    setClosed(true);
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawRRect(rrect(), paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipRRect(rrect(), clipOp);
  }

  void transform(const SkMatrix& matrix) override
  {
  }

  SkPath asPath() override
  {
    SkPath path;
    path.addRRect(m_rect);
    return path;
  }

  const SkRRect& rrect() const
  {
    return m_rect;
  }

private:
  SkRRect m_rect;
};

class RectShape : public ShapeBase
{
public:
  RectShape(const SkRect& rect)
  {
    m_rect = rect;
    setEmpty(false);
    setClosed(true);
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawRect(m_rect, paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipRect(m_rect, clipOp);
  }

  void transform(const SkMatrix& matrix) override
  {
    m_rect = matrix.mapRect(m_rect);
  }

  SkPath asPath() override
  {
    SkPath path;
    path.addRect(m_rect);
    return path;
  }

  SkRect rect() const
  {
    return m_rect;
  }

private:
  SkRect m_rect;
};

class EllipseShape : public ShapeBase
{
public:
  EllipseShape(const SkRect& rect)
  {
    m_oval = rect;
    setEmpty(false);
    setClosed(true);
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const override
  {
    canvas->drawOval(m_oval, paint);
  }
  void clip(SkCanvas* canvas, SkClipOp clipOp) const override
  {
    canvas->clipRect(m_oval, clipOp);
  }
  void transform(const SkMatrix& matrix) override
  {
    m_oval = matrix.mapRect(m_oval);
  }
  SkPath asPath() override
  {
    SkPath path;
    path.addOval(m_oval);
    return path;
  }

  SkRect ellipse() const
  {
    return m_oval;
  }

private:
  SkRect m_oval;
};

class ArcShape : public ShapeBase
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
  void transform(const SkMatrix& matrix) override
  {
    m_oval = matrix.mapRect(m_oval);
  }
  SkPath asPath() override
  {
    SkPath path;
    path.addOval(m_oval);
    return path;
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

  Shape()
    : m_type(EMPTY)
  {
  }

  ~Shape();

  Shape(const Shape& shape)
  {
    *this = shape;
  }

  Shape& operator=(const Shape& shape)
  {
    m_type = shape.m_type;
    m_impl = shape.m_impl;
    return *this;
  }

  bool isEmpty() const
  {
    return m_type == EMPTY || (m_type == PATH && m_impl->isEmpty());
  }

  explicit Shape(const SkPath& path)
  {
    setPath(path);
  }

  explicit Shape(ContourPtr contour)
  {
    setContour(contour);
  }

  explicit Shape(const SkRect& rect)
  {
    setRect(rect);
  }

  explicit Shape(const SkRRect& rrect)
  {
    setRRect(rrect);
  }

  explicit Shape(const Ellipse& oval)
  {
    setOval(oval);
  }

  explicit Shape(const Arc& arc)
  {
    setArc(arc.oval, arc.startAngle, arc.sweepAngle, arc.useCenter);
  }

  void setPath(const SkPath& path)
  {
    m_impl = std::make_shared<ContourShape>(path);
    m_type = PATH;
  }

  void setContour(const ContourPtr& contour)
  {
    m_impl = std::make_shared<ContourShape>(contour);
    m_type = PATH;
  }

  void setRect(const SkRect& rect)
  {
    m_impl = std::make_shared<RectShape>(rect);
    m_type = RECT;
  }

  std::optional<SkRect> asRect() const
  {
    if (type() == RECT)
    {
      return m_impl->asPath().getBounds();
    }
    return std::nullopt;
  }

  void setRRect(const SkRRect& rrect)
  {
    m_impl = std::make_shared<RRectShape>(rrect);
    m_type = RRECT;
  }

  std::optional<SkRRect> asRRect() const
  {
    if (type() == RRECT)
    {
      return std::static_pointer_cast<RRectShape>(m_impl)->rrect();
    }
    return std::nullopt;
  }

  void setArc(const SkRect& oval, float startAngle, float sweepAngle, bool forceMoveTo)
  {
    m_impl = std::make_shared<ArcShape>(oval, startAngle, sweepAngle, forceMoveTo);
    m_type = ARCH;
  }
  void setOval(const Ellipse& ellipse)
  {
    m_impl = std::make_shared<EllipseShape>(ellipse.rect);
    m_type = OVAL;
  }

  std::optional<Ellipse> asOval() const
  {
    if (type() == OVAL)
      return std::static_pointer_cast<EllipseShape>(m_impl)->ellipse();
    return std::nullopt;
  }

  void op(const Shape& shape, EBoolOp op);

  void transform(const SkMatrix& matrix)
  {
    m_impl->transform(matrix);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    m_impl->clip(canvas, clipOp);
  }

  void setFillType(EWindingType fillType)
  {
    if (this->type() == PATH)
    {
      std::static_pointer_cast<ContourShape>(m_impl)->setFillType(fillType);
    }
  }

  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    m_impl->draw(canvas, paint);
  }

  SkPath asPath() const
  {
    if (m_impl == nullptr)
      return SkPath();
    return m_impl->asPath();
  }

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
  std::shared_ptr<ShapeBase> m_impl;
  uint8_t                    m_type{ EMPTY };

}; // namespace VGG::layer
} // namespace VGG::layer
