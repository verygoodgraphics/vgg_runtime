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
#include "Layer/Core/PaintNode.hpp"
#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"

#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/matrix.hpp>
#include <core/SkCanvas.h>
#include <core/SkImage.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

extern std::unordered_map<std::string, sk_sp<SkImage>> g_skiaImageRepo;
namespace VGG
{

ResourceRepo Scene::s_resRepo{};

class Scene__pImpl
{
  VGG_DECL_API(Scene);

public:
  Scene__pImpl(Scene* api)
    : q_ptr(api)
  {
  }
  using RootArray = std::vector<FramePtr>;

  RootArray               roots;
  int                     page{ 0 };
  bool                    maskDirty{ true };
  Renderer                renderer;
  std::shared_ptr<Zoomer> zoomer;

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
    Frame* frame = roots[page];
    ASSERT(frame);
    if (maskDirty && frame)
    {
      renderer.updateMaskObject(frame->root());
      maskDirty = false;
    }
    if (frame)
    {
      frame->render(&renderer);
      q_ptr->onRenderFrame(canvas, frame->picture());
    }
  }
};

Scene::Scene()
  : d_ptr(new Scene__pImpl(this))
{
}
Scene::~Scene() = default;

void Scene::setSceneRoots(std::vector<FramePtr> roots)
{
  VGG_IMPL(Scene)
  if (roots.empty())
    return;
  _->page = 0;
  invalidateMask();
  _->roots = std::move(roots);
}

int Scene::currentPage() const
{
  return d_ptr->page;
}

void Scene::onRender(SkCanvas* canvas)
{
  VGG_IMPL(Scene)
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

void Scene::onRenderFrame(SkCanvas* canvas, SkPicture* frame)
{
  canvas->drawPicture(frame);
}

int Scene::frameCount() const
{
  return d_ptr->roots.size();
}

Frame* Scene::frame(int index)
{
  VGG_IMPL(Scene);
  if (index >= 0 && (std::size_t)index < _->roots.size())
  {
    auto f = _->roots[index];
    f->revalidate();
    return f;
  }
  return nullptr;
}
Zoomer* Scene::zoomer()
{
  VGG_IMPL(Scene)
  return _->zoomer.get();
}

void Scene::setZoomer(std::shared_ptr<Zoomer> zoomer)
{
  VGG_IMPL(Scene);
  if (_->zoomer)
  {
    _->zoomer->setOwnerScene(nullptr);
  }
  _->zoomer = std::move(zoomer);
  _->zoomer->setOwnerScene(this);
}

void Scene::setPage(int num)
{
  VGG_IMPL(Scene)
  if (num >= 0 && (std::size_t)num < _->roots.size())
  {
    _->page = num;
    invalidateMask();
  }
}

void Scene::onZoomChanged(float dx, float dy, float scale)
{
}
void Scene::invalidateMask()
{
  VGG_IMPL(Scene);
  _->maskDirty = true;
}

void Scene::invalidate()
{
  VGG_IMPL(Scene);
  for (auto& root : _->roots)
  {
    root->invalidate();
  }
}

void Scene::nextArtboard()
{
  VGG_IMPL(Scene)
  _->page = (_->page + 1 >= (int)_->roots.size()) ? _->page : _->page + 1;
  invalidateMask();
}

void Scene::preArtboard()
{
  VGG_IMPL(Scene)
  _->page = (_->page - 1 > 0) ? _->page - 1 : 0;
  invalidateMask();
}

void Scene::setResRepo(std::map<std::string, std::vector<char>> repo)
{
  Scene::s_resRepo = std::move(repo);
  g_skiaImageRepo.clear();
}

void Scene::enableDrawDebugBound(bool enabled)
{
  VGG_IMPL(Scene)
  invalidate();
  _->renderer.enableDrawDebugBound(enabled);
}
bool Scene::isEnableDrawDebugBound()
{
  VGG_IMPL(Scene)
  return _->renderer.isEnableDrawDebugBound();
}

} // namespace VGG
