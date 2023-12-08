/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "Layer/Core/Attrs.hpp"
#include "Layer/PathGenerator.hpp"
#include "Layer/VSkia.hpp"
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

template<typename SkType>
struct PrimitivePolicy
{
  SkType                        primitive;
  mutable std::optional<SkPath> cache;

public:
  PrimitivePolicy(const SkType& p)
    : primitive(p)
  {
  }
  void   draw(SkCanvas* canvas, const SkPaint& paint) const;
  void   clip(SkCanvas* canvas, SkClipOp clipOp) const;
  bool   isClosed() const;
  SkPath asPath() const;
};

struct Rect : PrimitivePolicy<SkRect>
{
  Rect(const Bound& rect)
    : PrimitivePolicy(toSkRect(rect))
  {
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    canvas->drawRect(primitive, paint);
  }
  bool isClosed() const
  {
    return true;
  };

  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    canvas->clipRect(primitive, clipOp);
  }
  SkPath asPath() const
  {
    if (!cache)
      cache = SkPath::Rect(primitive);
    return *cache;
  }
};

struct RoundRect : PrimitivePolicy<SkRRect>
{
  RoundRect(const SkRRect& rrect)
    : PrimitivePolicy(rrect)
  {
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    canvas->drawRRect(primitive, paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    canvas->clipRRect(primitive, clipOp);
  }
  SkPath asPath() const
  {
    if (!cache)
      cache = SkPath::RRect(primitive);
    return *cache;
  }
};

struct Oval : PrimitivePolicy<SkRect>
{
  Oval(const Bound& oval)
    : PrimitivePolicy(toSkRect(oval))
  {
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    canvas->drawOval(primitive, paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    canvas->clipPath(asPath(), clipOp);
  }

  SkPath asPath() const
  {
    if (!cache)
      cache = SkPath::Oval(primitive);
    return *cache;
  }
  bool isClosed() const
  {
    return true;
  };
};

struct Polygon : PrimitivePolicy<SkPoint*>
{
  bool closed{ false };
  Polygon(const SkPoint* polygon, int count, bool closed)
    : PrimitivePolicy(nullptr)
    , closed(closed)
  {
    cache = SkPath::Polygon(polygon, count, closed);
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    canvas->drawPath(asPath(), paint);
  }

  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    canvas->clipPath(asPath(), clipOp);
  }
  bool isClosed() const
  {
    return closed;
  };

  SkPath asPath() const
  {
    return *cache;
  }
};

struct Path : PrimitivePolicy<SkPath>
{
  bool closed{ false };
  Path(SkPath path)
    : PrimitivePolicy(std::move(path))
  {
  }
  void draw(SkCanvas* canvas, const SkPaint& paint) const
  {
    canvas->drawPath(primitive, paint);
  }
  bool isClosed() const
  {
    return primitive.isLastContourClosed();
  };
  void clip(SkCanvas* canvas, SkClipOp clipOp) const
  {
    canvas->clipPath(primitive, clipOp);
  }
  SkPath asPath() const
  {
    return primitive;
  }

  Bound bound() const
  {
    const auto r = primitive.getBounds();
    return Bound(r.left(), r.top(), r.width(), r.height());
  }
};

} // namespace VGG::layer
