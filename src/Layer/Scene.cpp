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
#include "Layer/SceneBuilder.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"

#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/matrix.hpp>
#include <core/SkCanvas.h>
#include <core/SkSurface.h>
#include <core/SkImage.h>

#include <filesystem>
#include <fstream>
#include <gpu/GpuTypes.h>
#include <gpu/GrDirectContext.h>
#include <gpu/GrRecordingContext.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <memory>
#include <string>

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
  sk_sp<SkImage>          rasterImage;
  std::shared_ptr<Zoomer> zoomer;

  std::optional<Bound> viewport;

  Bound onRevalidate() override
  {
    // only revalidate cuurent page
    rasterImage.reset();
    DEBUG("revalidate image");
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

  void ensureImage(SkCanvas* canvas, SkPicture* frame, const Bound& bound)
  {
    if (!rasterImage)
    {
      DEBUG("ensureImage");
      SkImageInfo info = SkImageInfo::MakeN32Premul(bound.width(), bound.height());
      auto        surface = canvas->makeSurface(info);
      if (surface)
      {
        surface->getCanvas()->drawPicture(frame);
        rasterImage = surface->makeImageSnapshot();
      }
    }
    ASSERT(rasterImage);
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
      auto viewportBound = viewport.value_or(frame->bound());
      auto mat = canvas->getTotalMatrix();
      frame->render(&renderer, &mat, &viewportBound);
      canvas->resetMatrix();
      if (!rasterImage)
      {
        rasterImage =
          q_ptr->onRasterFrame(canvas->recordingContext(), frame->picture(), viewportBound);
      }
      if (rasterImage)
      {
        canvas->drawImage(rasterImage, 0, 0);
      }
      canvas->setMatrix(mat);
    }
  }
};

void Scene::invalidateRasterImage()
{
  d_ptr->rasterImage = nullptr;
  invalidate();
}

Scene::Scene()
  : d_ptr(V_NEW<Scene__pImpl>(this))
{
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

sk_sp<SkImage> Scene::onRasterFrame(
  GrRecordingContext* canvas,
  SkPicture*          frame,
  const Bound&        bound)
{
  SkImageInfo info = SkImageInfo::MakeN32Premul(bound.width(), bound.height());
  auto        surface = SkSurfaces::RenderTarget(canvas, skgpu::Budgeted::kYes, info);
  if (surface)
  {
    surface->getCanvas()->drawPicture(frame);
    return surface->makeImageSnapshot();
  }
  DEBUG("Cannot create surface!");
  return nullptr;
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
  invalidate();
}

void Scene::onZoomTranslationChanged(float x, float y)
{
  invalidate();
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

void Scene::onViewportChange(const Bound& bound)
{
  d_ptr->viewport = bound;
}

bool Scene::isEnableDrawDebugBound()
{
  VGG_IMPL(Scene)
  return _->renderer.isEnableDrawDebugBound();
}

} // namespace VGG
