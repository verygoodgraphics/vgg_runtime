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
#include "Layer/Core/Attrs.hpp"
#include <core/SkClipOp.h>
#include <core/SkPaint.h>
#include <core/SkPath.h>
#include <core/SkPathTypes.h>
#include <core/SkRRect.h>
#include <glm/glm.hpp>
#include <optional>
#include <variant>

class SkCanvas;
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

class Shape;
class VShape
{
public:
  enum EShapeType : uint8_t
  {
    EMPTY,
    PATH,
    RECT,
    RRECT,
    ARC,
    OVAL
  };

  VShape();
  explicit VShape(const SkPath& path)
    : VShape()
  {
    setPath(path);
  }
  explicit VShape(ContourPtr contour)
    : VShape()
  {
    setContour(contour);
  }
  explicit VShape(const SkRect& rect)
    : VShape()
  {
    setRect(rect);
  }
  explicit VShape(const SkRRect& rrect)
    : VShape()
  {
    setRRect(rrect);
  }
  explicit VShape(const Ellipse& oval)
    : VShape()
  {
    setOval(oval);
  }
  explicit VShape(const Arc& arc)
    : VShape()
  {
    setArc(arc.oval, arc.startAngle, arc.sweepAngle, arc.useCenter);
  }

  ~VShape();
  VShape(const VShape& shape);
  VShape& operator=(const VShape& shape);

  void setPath(const SkPath& path);
  void setContour(const ContourPtr& contour);
  void setRect(const SkRect& rect);
  void setRRect(const SkRRect& rrect);
  void setArc(const SkRect& oval, float startAngle, float sweepAngle, bool forceMoveTo);
  void setOval(const Ellipse& ellipse);

  std::optional<SkRect>  asRect() const;
  std::optional<SkRRect> asRRect() const;
  std::optional<Ellipse> asOval() const;

  void op(const VShape& shape, EBoolOp op);
  void transform(VShape& shape, const SkMatrix& matrix);
  void clip(SkCanvas* canvas, SkClipOp clipOp) const;
  void draw(SkCanvas* canvas, const SkPaint& paint) const;

  void setFillType(EWindingType fillType);

  SkRect bounds() const;

  std::optional<VShape> outset(float x, float y) const;

  SkPath asPath() const;
  bool   isEmpty() const;
  bool   isClosed() const;

  EShapeType type() const
  {
    return m_type;
  }

  void reset();

private:
  std::unique_ptr<Shape> m_shape;
  EShapeType             m_type{ EMPTY };
};

std::variant<ContourPtr, SkRRect, SkRect> makeShape(
  std::array<float, 4> radius,
  const SkRect&        rect,
  float                cornerSmoothing);

} // namespace VGG::layer
