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

#include <utils/SkNWayCanvas.h>
#include <utils/SkNoDrawCanvas.h>

namespace VGG::layer
{

class DevCanvas : public SkNWayCanvas
{
public:
  DevCanvas(int width, int height)
    : SkNWayCanvas(width, height)
  {
    for (int i = 0; i < DRAW_CALL_TYPE; i++)
    {
      m_drawCall[i] = 0;
    }
  }
  int getDrawCall(int index) const
  {
    return m_drawCall[index];
  }

  int getTotalDrawCall() const
  {
    int total = 0;
    for (int i = 0; i < DRAW_CALL_TYPE; i++)
    {
      total += m_drawCall[i];
    }
    return total;
  }

  void resetDrawCall()
  {
    for (int i = 0; i < DRAW_CALL_TYPE; i++)
    {
      m_drawCall[i] = 0;
    }
  }

protected:
  void onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) override
  {
    SkNWayCanvas::onDrawAnnotation(rect, key, value);
    m_drawCall[DRAW_CALL_ANNOTATION]++;
  }
  void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawDRRect(outer, inner, paint);
    m_drawCall[DRAW_CALL_DRRECT]++;
  }
  void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override
  {
    SkNWayCanvas::onDrawDrawable(drawable, matrix);
    m_drawCall[DRAW_CALL_DRAWABLE]++;
  }
  void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawTextBlob(blob, x, y, paint);
    m_drawCall[DRAW_CALL_TEXTBLOB]++;
  }
  void onDrawPatch(
    const SkPoint  cubics[12],
    const SkColor  colors[4],
    const SkPoint  texCoords[4],
    SkBlendMode    bmode,
    const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawPatch(cubics, colors, texCoords, bmode, paint);
    m_drawCall[DRAW_CALL_PATCH]++;
  }

  void onDrawPaint(const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawPaint(paint);
    m_drawCall[DRAW_CALL_PAINT]++;
  }
  void onDrawBehind(const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawBehind(paint);
    m_drawCall[DRAW_CALL_BEHIND]++;
  }
  void onDrawPoints(PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint)
    override
  {
    SkNWayCanvas::onDrawPoints(mode, count, pts, paint);
    m_drawCall[DRAW_CALL_POINTS]++;
  }
  void onDrawRect(const SkRect& rect, const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawRect(rect, paint);
    m_drawCall[DRAW_CALL_RECT]++;
  }
  void onDrawRegion(const SkRegion& region, const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawRegion(region, paint);
    m_drawCall[DRAW_CALL_REGION]++;
  }
  void onDrawOval(const SkRect& rect, const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawOval(rect, paint);
    m_drawCall[DRAW_CALL_OVAL]++;
  }
  void onDrawArc(
    const SkRect&  rect,
    SkScalar       startAngle,
    SkScalar       sweepAngle,
    bool           useCenter,
    const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawArc(rect, startAngle, sweepAngle, useCenter, paint);
    m_drawCall[DRAW_CALL_ARC]++;
  }
  void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawRRect(rrect, paint);
    m_drawCall[DRAW_CALL_RRECT]++;
  }
  void onDrawPath(const SkPath& path, const SkPaint& paint) override
  {
    SkNWayCanvas::onDrawPath(path, paint);
    m_drawCall[DRAW_CALL_PATH]++;
  }

  void onDrawImage2(
    const SkImage*           image,
    SkScalar                 left,
    SkScalar                 top,
    const SkSamplingOptions& sampling,
    const SkPaint*           paint) override
  {
    m_drawCall[DRAW_CALL_IMAGE2]++;
  }
  void onDrawImageRect2(
    const SkImage*           image,
    const SkRect&            src,
    const SkRect&            dst,
    const SkSamplingOptions& sampling,
    const SkPaint*           paint,
    SrcRectConstraint        constraint) override
  {
    SkNWayCanvas::onDrawImageRect2(image, src, dst, sampling, paint, constraint);
    m_drawCall[DRAW_CALL_IMAGERECT2]++;
  }
  void onDrawImageLattice2(
    const SkImage* image,
    const Lattice& lattice,
    const SkRect&  dst,
    SkFilterMode   filter,
    const SkPaint* paint) override
  {
    SkNWayCanvas::onDrawImageLattice2(image, lattice, dst, filter, paint);
    m_drawCall[DRAW_CALL_IMAGELATTICE2]++;
  }
  void onDrawAtlas2(
    const SkImage*           image,
    const SkRSXform          xform[],
    const SkRect             tex[],
    const SkColor            colors[],
    int                      count,
    SkBlendMode              bmode,
    const SkSamplingOptions& sampling,
    const SkRect*            cull,
    const SkPaint*           paint) override
  {
    SkNWayCanvas::onDrawAtlas2(image, xform, tex, colors, count, bmode, sampling, cull, paint);
    m_drawCall[DRAW_CALL_ATLAS2]++;
  }

  void onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode, const SkPaint& paint)
    override
  {
    SkNWayCanvas::onDrawVerticesObject(vertices, bmode, paint);
    m_drawCall[DRAW_CALL_VERTICESOBJECT]++;
  }
  void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override
  {
    SkNWayCanvas::onDrawShadowRec(path, rec);
    m_drawCall[DRAW_CALL_SHADOWREC]++;
  }
  void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override
  {
    SkNWayCanvas::onDrawPicture(nullptr, nullptr, nullptr);
    m_drawCall[DRAW_CALL_PICTURE]++;
  }

  void onDrawEdgeAAQuad(
    const SkRect&    rect,
    const SkPoint    clip[4],
    QuadAAFlags      aa,
    const SkColor4f& color,
    SkBlendMode      mode) override
  {
    SkNWayCanvas::onDrawEdgeAAQuad(rect, clip, aa, color, mode);
    m_drawCall[DRAW_CALL_EDGEAAQUAD]++;
  }
  void onDrawEdgeAAImageSet2(
    const ImageSetEntry      set[],
    int                      count,
    const SkPoint            dstClips[],
    const SkMatrix           preViewMatrices[],
    const SkSamplingOptions& sampling,
    const SkPaint*           paint,
    SrcRectConstraint        constraint) override
  {
    SkNWayCanvas::onDrawEdgeAAImageSet2(
      set,
      count,
      dstClips,
      preViewMatrices,
      sampling,
      paint,
      constraint);
    m_drawCall[DRAW_CALL_EDGEAAIMAGESET2]++;
  }

private:
  static constexpr int DRAW_CALL_TYPE = 25;
  enum EDrawCallIndex
  {
    DRAW_CALL_ANNOTATION = 0,
    DRAW_CALL_DRRECT,
    DRAW_CALL_DRAWABLE,
    DRAW_CALL_TEXTBLOB,
    DRAW_CALL_PATCH,
    DRAW_CALL_PAINT,
    DRAW_CALL_BEHIND,
    DRAW_CALL_POINTS,
    DRAW_CALL_RECT,
    DRAW_CALL_REGION,
    DRAW_CALL_OVAL,
    DRAW_CALL_ARC,
    DRAW_CALL_RRECT,
    DRAW_CALL_PATH,
    DRAW_CALL_IMAGE2,
    DRAW_CALL_IMAGERECT2,
    DRAW_CALL_IMAGELATTICE2,
    DRAW_CALL_ATLAS2,
    DRAW_CALL_VERTICESOBJECT,
    DRAW_CALL_SHADOWREC,
    DRAW_CALL_PICTURE,
    DRAW_CALL_EDGEAAQUAD,
    DRAW_CALL_EDGEAAIMAGESET2
  };
  int m_drawCall[DRAW_CALL_TYPE];
};
} // namespace VGG::layer
