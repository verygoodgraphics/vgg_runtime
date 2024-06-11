#include "RasterTileNode.hpp"

#include "Layer/Raster.hpp"
#include "Layer/Core/RasterCacheTile.hpp"
#include "Layer/Core/ViewportNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Renderer.hpp"

namespace
{

using namespace VGG::layer;
inline std::optional<Rasterizer::EReason> changed(glm::mat3& prevMatrix, const glm::mat3& newMatrix)
{
  if (prevMatrix[0][0] != newMatrix[0][0] || prevMatrix[1][1] != newMatrix[1][1])
  {
    return Rasterizer::EReason::ZOOM_SCALE;
  }
  else if (prevMatrix[2][0] != newMatrix[2][0] || prevMatrix[2][1] != newMatrix[2][1])
  {
    return Rasterizer::EReason::ZOOM_TRANSLATION;
  }
  return std::nullopt;
}
} // namespace
namespace VGG::layer
{

TileRasterNode::TileRasterNode(
  VRefCnt*            cnt,
  GrRecordingContext* device,
  RasterExecutor*     executor,
  Ref<Viewport>       viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     child)
  : RasterNode(cnt, device, executor, std::move(viewport), std::move(zoomer), std::move(child))
  , m_raster(std::make_unique<RasterCacheTile>())
{
  ASSERT(getChild());
  ASSERT(m_raster);
  observe(viewport);
#ifdef VGG_LAYER_DEBUG
  dbgInfo = "RasterNode";
#endif
}

void TileRasterNode::render(Renderer* renderer)
{
  auto c = getChild();
  ASSERT(c);
  if (!c->picture())
  {
    TransformEffectNode::render(renderer);
  }
  else
  {
    auto canvas = renderer->canvas();
    ASSERT(canvas);
    canvas->save();
    canvas->resetMatrix();
    canvas->setMatrix(m_rasterMatrix);
    for (auto& tile : m_rasterTiles)
    {
      canvas->drawImage(tile.image, tile.rect.left(), tile.rect.top());
      // SkPaint p;
      // p.setColor(SK_ColorRED);
      // p.setStyle(SkPaint::kStroke_Style);
      // canvas->drawRect(tile.rect, p);
    }
    canvas->restore();
  }
}

#ifdef VGG_LAYER_DEBUG
void TileRasterNode::debug(Renderer* render)
{
  auto c = getChild();
  ASSERT(c);
  if (c)
  {
    auto                z = getTransform();
    SkAutoCanvasRestore acr(render->canvas(), true);
    render->canvas()->concat(toSkMatrix(z->getMatrix()));
    c->debug(render);
  }
}
#endif

Bounds TileRasterNode::onRevalidate(Revalidation* inv, const glm::mat3& mat)
{
  TransformEffectNode::onRevalidate(inv, mat);
  auto   c = getChild();
  bool   needRaster = false;
  Bounds finalBounds;

  // FIXME: you cannot determine the invalidation by hasInval**() functions,
  // because it could be revaildated by other nodes. unless it is exclusive to this node.
  //
  // TODO:: This object does not have to be ZoomerNode-awared, it can be achieved by carefully
  // judging from coressponding component of matrix in TransformNode.  In other words, it should
  // be able to handle any sort of transformation node.
  if (m_raster)
  {
    if (m_cacheUniqueID != c->picture()->uniqueID())
    {
      DEBUG("content changed");
      m_cacheUniqueID = c->picture()->uniqueID();
      m_raster->invalidate(layer::Rasterizer::EReason::CONTENT);
      m_matrix = getTransform()->getMatrix();
      needRaster = true;
    }
    else
    {
      if (true)
      {
        auto newMatrix = getTransform()->getMatrix();
        auto changeType = changed(m_matrix, newMatrix);
        if (changeType)
        {
          m_raster->invalidate(*changeType);
          m_matrix = newMatrix;
          needRaster = true;
        }
        else
        {
          m_raster->invalidate(layer::Rasterizer::EReason::CONTENT);
          needRaster = true;
        }
        if (viewport() && viewport()->hasInvalidate())
        {
          m_raster->invalidate(layer::Rasterizer::EReason::VIEWPORT);
          viewport()->revalidate();
          needRaster = true;
          finalBounds = viewport()->bounds();
        }
      }
      else
      {
        // auto z = asZoom(getTransform());
        // if (z)
        // {
        //   if (z->hasInvalScale())
        //   {
        //     DEBUG("zoom scale changed");
        //     m_raster->invalidate(layer::Rasterizer::EReason::ZOOM_SCALE);
        //     needRaster = true;
        //   }
        //   if (z->hasOffsetInval())
        //   {
        //     DEBUG("zoom translation changed");
        //     m_raster->invalidate(layer::Rasterizer::EReason::ZOOM_TRANSLATION);
        //     needRaster = true;
        //   }
        //   z->revalidate();
        // }
        // if (m_viewport && m_viewport->hasInvalidate())
        // {
        //   m_raster->invalidate(layer::Rasterizer::EReason::VIEWPORT);
        //   m_viewport->revalidate();
        //   needRaster = true;
        //   finalBounds = m_viewport->bounds();
        // }
      }
    }
  }

  if (needRaster)
  {
    SkRect skv = toSkRect(getChild()->bounds());
    if (viewport())
    {
      skv = toSkRect(viewport()->getViewport());
    }

    int lod = -1;
    // auto z = asZoom(getTransform());
    // if (z)
    // {
    //   std::visit(
    //     layer::Overloaded{ [&](ZoomerNode::EScaleLevel level) { lod = level; },
    //                        [&](ZoomerNode::OtherLevel other) { lod = -1; } },
    //     z->scaleLevel());
    // }

    if (c->picture())
    {
      const auto                localMatrix = SkMatrix::I();
      const auto                deviceMatrix = toSkMatrix(getTransform()->getMatrix());
      const auto                contentBounds = toSkRect(c->bounds());
      Rasterizer::RasterContext rasterCtx{ deviceMatrix,
                                           c->picture(),
                                           &contentBounds,
                                           localMatrix };
      m_raster->rasterize(device(), rasterCtx, lod, skv, &m_rasterTiles, &m_rasterMatrix, 0);
    }
  }
  return finalBounds;
}

namespace raster
{

Ref<RasterNode> makeTileRaster(
  GrRecordingContext* device,
  Ref<Viewport>       viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     renderObject)
{
  return TileRasterNode::Make(
    device,
    nullptr,
    std::move(viewport),
    std::move(zoomer),
    std::move(renderObject));
}

} // namespace raster
} // namespace VGG::layer
