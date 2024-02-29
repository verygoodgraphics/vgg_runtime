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
#include <core/SkPaint.h>
#include <core/SkPath.h>
#include <core/SkPathTypes.h>
#include <core/SkRRect.h>
#include <core/SkClipOp.h>
class SkCanvas;
namespace VGG::layer
{

class Shape
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

  virtual Shape* clone() const = 0;

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
} // namespace VGG::layer
