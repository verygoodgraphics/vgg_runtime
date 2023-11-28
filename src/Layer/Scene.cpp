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
#include "DocBuilder.hpp"
#include "Renderer.hpp"

#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"
#include "Layer/Scene.hpp"
#include "Renderer.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Core/PaintNode.hpp"

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
  using RootArray = std::vector<std::shared_ptr<layer::PaintNode>>;

  RootArray               roots;
  int                     page{ 0 };
  int                     symbolIndex{ 0 };
  bool                    maskDirty{ true };
  SkiaRenderer            renderer;
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
    layer::PaintNode* node = roots[page].get();
    if (!roots.empty() && maskDirty && node)
    {
      renderer.clearCache();
      renderer.updateMaskObject(node);
      maskDirty = false;
    }
    if (node)
    {
      renderer.draw(canvas, node);
    }
    // renderer.commit(canvas);
  }
};

Scene::Scene()
  : d_ptr(new Scene__pImpl(this))
{
}
void Scene::loadFileContent(const std::string& json)
{
  loadFileContent(nlohmann::json::parse(json));
}
Scene::~Scene() = default;

void Scene::loadFileContent(const nlohmann::json& json)
{
  VGG_IMPL(Scene)
  if (json.empty())
    return;
  _->page = 0;
  _->symbolIndex = 0;
  repaint();
  _->roots = layer::DocBuilder::build(json);
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

int Scene::frameCount() const
{
  return d_ptr->roots.size();
}

PaintNode* Scene::frame(int index)
{
  VGG_IMPL(Scene);
  if (index >= 0 && index < _->roots.size())
  {
    return _->roots[index].get();
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
  if (num >= 0 && num < _->roots.size())
  {
    _->page = num;
    repaint();
  }
}

void Scene::repaint()
{
  VGG_IMPL(Scene);
  _->maskDirty = true;
}

void Scene::nextArtboard()
{
  VGG_IMPL(Scene)
  _->page = (_->page + 1 >= _->roots.size()) ? _->page : _->page + 1;
  repaint();
}

void Scene::preArtboard()
{
  VGG_IMPL(Scene)
  _->page = (_->page - 1 > 0) ? _->page - 1 : 0;
  repaint();
}

void Scene::nextSymbol()
{
  VGG_IMPL(Scene)
  // _->symbolIndex =
  //   (_->symbolIndex + 1 >= _->container.symbols.size()) ? _->symbolIndex : _->symbolIndex + 1;
}

void Scene::prevSymbol()
{
  VGG_IMPL(Scene)
  _->symbolIndex = (_->symbolIndex - 1 > 0) ? _->symbolIndex - 1 : 0;
}

void Scene::setResRepo(std::map<std::string, std::vector<char>> repo)
{
  Scene::s_resRepo = std::move(repo);
  g_skiaImageRepo.clear();
}

void Scene::enableDrawDebugBound(bool enabled)
{
  VGG_IMPL(Scene)
  _->renderer.enableDrawDebugBound(enabled);
}
bool Scene::isEnableDrawDebugBound()
{
  VGG_IMPL(Scene)
  return _->renderer.isEnableDrawDebugBound();
}

} // namespace VGG
