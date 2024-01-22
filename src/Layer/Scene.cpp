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

#include "Renderer.hpp"

#include "Layer/Scene.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/RasterCacheImpl.hpp"
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

extern std::unordered_map<std::string, sk_sp<SkImage>> g_skiaImageRepo;
namespace VGG
{

ResourceRepo Scene::s_resRepo{};

class Scene__pImpl : public VNode
{
  VGG_DECL_API(Scene);

public:
  Scene__pImpl(VRefCnt* cnt, Scene* api)
    : VNode(cnt)
    , q_ptr(api)
  {
  }
  using RootArray = std::vector<FramePtr>;

  RootArray               roots;
  int                     page{ 0 };
  bool                    maskDirty{ true };
  Renderer                renderer;
  std::shared_ptr<Zoomer> zoomer;

  std::optional<Bound> viewport;

  std::unique_ptr<Rasterizer>        cache;
  std::optional<Rasterizer::EReason> reason;

  Bound onRevalidate() override
  {
    // only revalidate cuurent page
    DEBUG("revalidate image");
    if (cache)
      cache->invalidate(Rasterizer::EReason::CONTENT);
    if (auto f = currentFrame(true); f)
      return f->bound();
    else
      return bound();
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

  Frame* currentFrame(bool revalidate = false)
  {
    Frame* f = nullptr;
    if (page >= 0 && page < (int)roots.size())
    {
      f = roots[page].get();
      if (f && revalidate)
        f->revalidate();
    }
    return f;
  }

  Frame* frame(int index, bool revalidate = false)
  {
    Frame* f = nullptr;
    if (index >= 0 && index < (int)roots.size())
    {
      f = roots[index].get();
      if (f && revalidate)
        f->revalidate();
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
    maskDirty = true;
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
    canvas->translate(offset.x, offset.y);
    canvas->scale(zoom, zoom);
  }

  void restoreZoom(SkCanvas* canvas)
  {
    ASSERT(canvas);
    ASSERT(zoomer);
    auto offset = zoomer->translate();
    auto zoom = zoomer->scale();
    canvas->scale(1. / zoom, 1. / zoom);
    canvas->translate(-offset.x, -offset.y);
  }

  void render(SkCanvas* canvas)
  {
    if (roots.empty())
      return;
    auto* frame = currentFrame(true);
    ASSERT(frame);
    if (maskDirty && frame)
    {
      renderer.updateMaskObject(frame->root());
      maskDirty = false;
    }
    if (frame)
    {
      auto mat = canvas->getTotalMatrix(); // DPI * zoom
      auto context = canvas->recordingContext();
      frame->render(&renderer, nullptr);
      if (cache)
      {
        auto              skv = toSkRect(viewport.value_or(frame->bound()));
        auto              skr = toSkRect(frame->bound());
        auto              skm = toSkMatrix(frame->transform().matrix());
        Rasterizer::Key   key{ context, &mat, skv, frame->picture(), skr, skm, 0 };
        Rasterizer::Tile* tile = nullptr;
        int               count = 0;
        cache->rasterize(key, &tile, &count, &mat);
        canvas->save();
        canvas->resetMatrix();
        canvas->setMatrix(mat);
        for (int i = 0; i < count; i++)
        {
          canvas->drawImage(tile[i].image, tile[i].rect.left(), tile[i].rect.top());
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

Scene::Scene(std::unique_ptr<Rasterizer> cache)
  : d_ptr(V_NEW<Scene__pImpl>(this))
{
  if (cache)
  {
    d_ptr->cache = std::move(cache);
  }
}
Scene::~Scene() = default;

void Scene::setSceneRoots(std::vector<FramePtr> roots)
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

Frame* Scene::frame(int index)
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
  VGG_IMPL(Scene)
  if (num >= 0 && (std::size_t)num < _->roots.size())
  {
    _->page = num;
    invalidate();
    invalidateMask();
  }
}

void Scene::onZoomScaleChanged(float scale)
{
  DEBUG("Scene: onZoomScaleChanged");
  if (d_ptr->cache)
    d_ptr->cache->invalidate(Rasterizer::EReason::ZOOM_SCALE);
}

void Scene::onZoomTranslationChanged(float x, float y)
{
  DEBUG("Scene: onZoomTranslationChanged");
  if (d_ptr->cache)
    d_ptr->cache->invalidate(Rasterizer::EReason::ZOOM_TRANSLATION);
}

void Scene::onZoomViewportChanged(const Bound& bound)
{
  DEBUG("Scene: onZoomViewportChanged");
  if (d_ptr->cache)
    d_ptr->cache->invalidate(Rasterizer::EReason::VIEWPORT);
}

void Scene::onViewportChange(const Bound& bound)
{
  d_ptr->viewport = bound;
  if (d_ptr->cache)
    d_ptr->cache->invalidate(Rasterizer::EReason::VIEWPORT);
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
  g_skiaImageRepo.clear();
}

void Scene::enableDrawDebugBound(bool enabled)
{
  VGG_IMPL(Scene)
  _->invalidateCurrentFrame();
  _->renderer.enableDrawDebugBound(enabled);
}

bool Scene::isEnableDrawDebugBound()
{
  VGG_IMPL(Scene)
  return _->renderer.isEnableDrawDebugBound();
}

} // namespace VGG
