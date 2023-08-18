#include "Scene/Scene.h"
#include "Application/include/Event/Event.h"
#include "Core/Node.h"
#include "Reader/Loader.h"
#include "Core/PaintNode.h"
#include "Scene/Renderer.h"
#include "Scene/Zoomer.h"
#include "core/SkCanvas.h"
#include "core/SkImage.h"
#include <filesystem>
#include "ConfigMananger.h"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/matrix.hpp"
#include <fstream>
#include <memory>
#include <string>

extern std::unordered_map<std::string, sk_sp<SkImage>> g_skiaImageRepo;
namespace VGG
{

ResourceRepo Scene::s_resRepo{};
ObjectTableType Scene::s_objectTable{};
ObjectTableType Scene::s_templateObjectTable{};
InstanceTable Scene::s_instanceTable{};
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
  bool renderSymbol{ false };
  bool maskDirty{ true };
  std::shared_ptr<ZoomerListener> zoomer;

  void render(SkCanvas* canvas)
  {
    PaintNode* node = nullptr;
    SkiaRenderer r;
    if (!renderSymbol)
    {
      if (!container.frames.empty())
      {
        auto board = container.frames[page].get();
        preprocessMask(board);
        r.draw(canvas, board);
      }
    }
    else
    {
      if (!container.symbols.empty())
      {
        node = container.symbols[symbolIndex].get();
        r.draw(canvas, node);
      }
    }
  }

  void preprocessMask(PaintNode* node)
  {
    if (maskDirty)
    {
      Scene::s_objectTable = node->preprocessMask();
      // generate each mask for masked node
      maskDirty = false;
    }
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

void Scene::instantiateTemplates()
{
  for (auto& p : Scene::instanceObjects())
  {
    if (auto node = p.second.first.lock(); node)
    {
      auto& templates = Scene::templateObjectTable();
      if (auto it = templates.find(p.second.second); it != templates.end())
      {
        if (auto master = it->second.lock(); master)
        {
          auto instance = std::static_pointer_cast<PaintNode>(master->cloneChildren());

          auto transform = instance->localTransform();
          // clear transform
          transform = glm::translate(transform, glm::vec2{ -transform[2][0], -transform[2][1] });
          instance->setLocalTransform(transform);
          // TODO:: override properties
          node->addChild(instance);
        }
        else
        {
          ASSERT_MSG(false, "master node is expired");
        }
      }
      else
      {
        DEBUG("symbol master [%s] doesn't exist referenced by [%s]",
              p.second.second.c_str(),
              p.first.c_str());
      }
    }
  }
}

void Scene::loadFileContent(const nlohmann::json& json)
{
  VGG_IMPL(Scene)
  if (json.empty())
    return;
  s_templateObjectTable.clear();
  s_instanceTable.clear();
  _->page = 0;
  _->symbolIndex = 0;
  _->maskDirty = true;
  _->container = NlohmannBuilder::build(json);
  for (const auto& s : _->container.symbols)
  {
    s_templateObjectTable[s->guid()] = s;
  }
  instantiateTemplates();
}

bool Scene::dispatchEvent(UEvent e, void* userData)
{
  VGG_IMPL(Scene)
  if (_->zoomer)
  {
    _->zoomer->dispatchEvent(e, this);
  }
  if (e.type == VGG_PAINT)
  {
    onPaintEvent(e.paint);
  }
  return true;
}

bool Scene::onPaintEvent(VPaintEvent e)
{
  VGG_IMPL(Scene)
  auto canvas = (SkCanvas*)e.data;
  // handle zooming
  _->render(canvas);
  // handle zooming
  return true;
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

void Scene::setZoomer(std::shared_ptr<ZoomerListener> zoomer)
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
