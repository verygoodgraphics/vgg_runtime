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

#include "Layer/Core/RasterCache.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/LayerCache.h"
#include "Renderer.hpp"
#include "Settings.hpp"

#include "Layer/Scene.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/Timer.hpp"
#include "encode/SkPngEncoder.h"
#include "Utility/Log.hpp"

#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <encode/SkPngEncoder.h>
#include <core/SkSurface.h>
#include <core/SkImage.h>

#include <gpu/GpuTypes.h>
#include <gpu/GrDirectContext.h>
#include <gpu/GrRecordingContext.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <memory>
#include <string>
#include <fstream>
#include <variant>

namespace VGG
{

ResourceRepo Scene::s_resRepo{};

class Scene__pImpl : public layer::VNode
{
  VGG_DECL_API(Scene);

public:
  Scene__pImpl(layer::VRefCnt* cnt, Scene* api)
    : VNode(cnt)
    , q_ptr(api)
  {
  }
  using RootArray = std::vector<layer::FramePtr>;

  RootArray               roots;
  int                     page{ 0 };
  layer::Renderer         renderer;
  std::shared_ptr<Zoomer> zoomer;
  std::optional<Bounds>   viewport;

  std::unique_ptr<layer::Rasterizer>   rasterizer;
  std::vector<layer::Rasterizer::Tile> rasterTiles;
  SkMatrix                             rasterMatrix;
  int                                  lod{ -1 };

  Bounds onRevalidate() override
  {
    // only revalidate cuurent page
    if (rasterizer)
      rasterizer->invalidate(layer::Rasterizer::EReason::CONTENT);
    if (auto f = currentFrame(true); f)
    {
      return f->bounds();
    }
    else
    {
      return bounds();
    }
  }

  void setFrames(RootArray roots)
  {
    if (!this->roots.empty())
    {
      for (const auto& r : this->roots)
      {
        unobserve(r);
      }
    }
    this->roots = std::move(roots);
    for (const auto& r : this->roots)
    {
      observe(r);
    }
    page = 0;
    invalidate();
    invalidateMask();
  }

  layer::Frame* currentFrame(bool revalidate = false)
  {
    layer::Frame* f = nullptr;
    if (page >= 0 && page < (int)roots.size())
    {
      f = roots[page].get();
      if (f && revalidate)
      {
        f->revalidate();
      }
    }
    return f;
  }

  layer::Frame* frame(int index, bool revalidate = false)
  {
    layer::Frame* f = nullptr;
    if (index >= 0 && index < (int)roots.size())
    {
      f = roots[index].get();
      if (f && revalidate)
      {
        f->revalidate();
      }
    }
    return f;
  }

  void invalidateAllFrames()
  {
    for (auto& f : this->roots)
      f->invalidate();
  }

  void invalidateCurrentFrame()
  {
    auto f = currentFrame(false);
    if (f)
      f->invalidate();
  }

  void nextFrame()
  {
    page = (page + 1 >= (int)roots.size()) ? page : page + 1;
    invalidate();
    invalidateMask();
  }

  void invalidateMask()
  {
    auto f = currentFrame(false);
    f->invalidateMask();
  }

  void preFrame()
  {
    page = (page - 1 > 0) ? page - 1 : 0;
    invalidate();
    invalidateMask();
  }

  void applyZoom(SkCanvas* canvas)
  {
    ASSERT(canvas);
    ASSERT(zoomer);
    auto offset = zoomer->translate();
    auto zoom = zoomer->scale();
    canvas->save();
    canvas->translate(offset.x, offset.y);
    canvas->scale(zoom, zoom);
  }

  void restoreZoom(SkCanvas* canvas)
  {
    ASSERT(canvas);
    ASSERT(zoomer);
    canvas->restore();
  }

  void render(SkCanvas* canvas)
  {
    if (roots.empty())
      return;
    auto* frame = currentFrame(true);
    ASSERT(frame);
    if (frame)
    {
      if (rasterizer)
      {
        if (rasterizer->isInvalidate())
        {
          auto                             rasterDevice = canvas->recordingContext();
          const auto                       skv = toSkRect(viewport.value_or(frame->bounds()));
          auto                             mat = canvas->getTotalMatrix(); // DPI * zoom
          const auto                       skr = toSkRect(frame->bounds());
          const auto                       skm = toSkMatrix(frame->transform().matrix());
          layer::Rasterizer::RasterContext rasterCtx{ mat, frame->picture(), &skr, skm };
          rasterizer->rasterize(rasterDevice, rasterCtx, lod, skv, &rasterTiles, &rasterMatrix, 0);
        }
        canvas->save();
        canvas->resetMatrix();
        canvas->setMatrix(rasterMatrix);
        for (auto& tile : rasterTiles)
        {
          canvas->drawImage(tile.image, tile.rect.left(), tile.rect.top());
          // SkPaint p;
          // p.setColor(SK_ColorRED);
          // p.setStyle(SkPaint::kStroke_Style);
          // canvas->drawRect(tile[i].rect, p);
        }
        canvas->restore();
      }
      else
      {
        canvas->save();
        canvas->concat(toSkMatrix(frame->transform().matrix()));
        canvas->drawPicture(frame->picture());
        canvas->restore();
      }
    }
  }
};

Scene::Scene(std::unique_ptr<layer::Rasterizer> cache)
  : d_ptr(V_NEW<Scene__pImpl>(this))
{
  if (cache)
  {
    d_ptr->rasterizer = std::move(cache);
  }
}
Scene::~Scene() = default;

void Scene::setSceneRoots(std::vector<layer::FramePtr> roots)
{
  VGG_IMPL(Scene)
  if (roots.empty())
    return;
  _->setFrames(std::move(roots));
}

int Scene::currentPage() const
{
  return d_ptr->page;
}

void Scene::onRender(SkCanvas* canvas)
{
  VGG_IMPL(Scene)
  _->revalidate();
  if (_->zoomer)
  {
    // handle zooming
    _->applyZoom(canvas);
    _->render(canvas);
    _->restoreZoom(canvas);
  }
  else
  {
    _->render(canvas);
  }
}

int Scene::frameCount() const
{
  return d_ptr->roots.size();
}

layer::Frame* Scene::frame(int index)
{
  VGG_IMPL(Scene);
  return _->frame(index, true);
}

Zoomer* Scene::zoomer()
{
  VGG_IMPL(Scene)
  return _->zoomer.get();
}

void Scene::setZoomer(std::shared_ptr<Zoomer> zoomer)
{
  VGG_IMPL(Scene);
  if (_->zoomer == zoomer)
    return;

  if (_->zoomer)
  {
    _->zoomer->setOwnerScene(nullptr);
  }
  _->zoomer = std::move(zoomer);
  _->zoomer->setOwnerScene(this);
  invalidate();
}

void Scene::setPage(int num)
{
  DEBUG("setPage: %d", num);
  VGG_IMPL(Scene)
  if (num >= 0 && (std::size_t)num < _->roots.size())
  {
    _->page = num;
    invalidate();
    invalidateMask();
  }
}

void Scene::onZoomScaleChanged(Zoomer::Scale value)
{
  DEBUG("Scene: onZoomScaleChanged");
  std::visit(
    layer::Overloaded{ [this](Zoomer::EScaleLevel level) { this->d_ptr->lod = level; },
                       [this](Zoomer::OtherLevel other) { this->d_ptr->lod = -1; } },
    value.first);
  if (d_ptr->rasterizer)
    d_ptr->rasterizer->invalidate(layer::Rasterizer::EReason::ZOOM_SCALE);
}

void Scene::onZoomTranslationChanged(float x, float y)
{
  DEBUG("Scene: onZoomTranslationChanged");
  if (d_ptr->rasterizer)
    d_ptr->rasterizer->invalidate(layer::Rasterizer::EReason::ZOOM_TRANSLATION);
}

void Scene::onViewportChange(const Bounds& bounds)
{
  d_ptr->viewport = bounds;
  if (d_ptr->rasterizer)
    d_ptr->rasterizer->invalidate(layer::Rasterizer::EReason::VIEWPORT);
}

void Scene::invalidateMask()
{
  VGG_IMPL(Scene);
  _->invalidateMask();
}

void Scene::invalidate()
{
  VGG_IMPL(Scene);
  auto f = _->currentFrame(false);
  if (f)
    f->invalidate();
}

void Scene::nodeAt(int x, int y, layer::PaintNode::NodeVisitor visitor)
{
  if (auto f = d_ptr->currentFrame(true); f)
  {
    if (d_ptr->zoomer)
    {
      const auto fp = d_ptr->zoomer->invMatrix() * glm::vec3{ x, y, 1 };
      x = fp.x;
      y = fp.y;
    }
    f->nodeAt(x, y, visitor);
  }
}

void Scene::nextArtboard()
{
  VGG_IMPL(Scene)
  _->nextFrame();
}

void Scene::preArtboard()
{
  VGG_IMPL(Scene)
  _->preFrame();
}

void Scene::setResRepo(std::map<std::string, std::vector<char>> repo)
{
  Scene::s_resRepo = std::move(repo);
  layer::getGlobalImageCache()->purge();
}

void Scene::enableDrawDebugBounds(bool enabled)
{
  VGG_IMPL(Scene)
  _->invalidateCurrentFrame();
  layer::enableDebugBounds(enabled);
}

bool Scene::isEnableDrawDebugBounds()
{
  return layer::getDebugBoundsEnable();
}

} // namespace VGG
