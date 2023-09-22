#include "Loader.hpp"
#include "Renderer.hpp"

#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"
#include "VGG/Layer/Scene.hpp"
#include "Renderer.hpp"
#include "VGG/Layer/Zoomer.hpp"
#include "VGG/Layer/Core/PaintNode.hpp"
#include "VGG/Layer/Core/Node.hpp"

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
ObjectTableType Scene::s_objectTable{};
bool Scene::s_enableDrawDebugBound{ false };

class Scene__pImpl
{
  VGG_DECL_API(Scene);

public:
  Scene__pImpl(Scene* api)
    : q_ptr(api)
  {
  }
  NodeContainer container;
  int page{ 0 };
  int symbolIndex{ 0 };
  bool maskDirty{ true };
  SkiaRenderer renderer;
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
    PaintNode* node = nullptr;
    if (!container.frames.empty() && maskDirty)
    {
      auto board = container.frames[page].get();
      preprocessMask(board);
      renderer.draw(canvas, board);
      maskDirty = false;
    }
    if (!maskDirty)
    {
      renderer.commit(canvas);
    }
  }

  void preprocessMask(PaintNode* node)
  {
    Scene::s_objectTable = node->preprocessMask();
    // generate each mask for masked node
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
  _->maskDirty = true;
  _->container = NlohmannBuilder::build(json);
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
  return d_ptr->container.frames.size();
}

PaintNode* Scene::frame(int index)
{
  VGG_IMPL(Scene);
  if (index >= 0 && index < _->container.frames.size())
  {
    return _->container.frames[index].get();
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
  _->zoomer = std::move(zoomer);
}

void Scene::setPage(int num)
{
  VGG_IMPL(Scene)
  if (num >= 0 && num < _->container.frames.size())
  {
    _->page = num;
    _->maskDirty = true;
  }
}

void Scene::nextArtboard()
{
  VGG_IMPL(Scene)
  _->page = (_->page + 1 >= _->container.frames.size()) ? _->page : _->page + 1;
  _->maskDirty = true;
}

void Scene::preArtboard()
{
  VGG_IMPL(Scene)
  _->page = (_->page - 1 > 0) ? _->page - 1 : 0;
  _->maskDirty = true;
}

void Scene::nextSymbol()
{
  VGG_IMPL(Scene)
  _->symbolIndex =
    (_->symbolIndex + 1 >= _->container.symbols.size()) ? _->symbolIndex : _->symbolIndex + 1;
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

} // namespace VGG
