#include "Scene/Scene.h"

#include "Core/FontManager.h"
#include "Reader/Loader.h"
#include "Core/PaintNode.h"
#include "Scene/Renderer.h"
#include "core/SkCanvas.h"
#include <filesystem>
#include "ConfigMananger.h"
#include <fstream>
#include <string>
namespace VGG
{

ResourceRepo Scene::ResRepo{};
ObjectTableType Scene::ObjectTable{};
bool Scene::s_enableDrawDebugBound{ false };

Scene::Scene()
{
  auto& fontMgr = FontManager::instance();
}
void Scene::loadFileContent(const std::string& json)
{
  loadFileContent(nlohmann::json::parse(json));
}

void Scene::loadFileContent(const nlohmann::json& json)
{
  if (json.empty())
    return;
  artboards = NlohmannBuilder::fromArtboard(json);
  symbols = NlohmannBuilder::fromSymbolMasters(json);
  page = 0;
  symbolIndex = 0;
  maskDirty = true;
}

void Scene::render(SkCanvas* canvas)
{
  PaintNode* node = nullptr;
  SkiaRenderer r;
  if (!renderSymbol)
  {
    if (!artboards.empty())
    {
      auto board = artboards[page].get();
      preprocessMask(board);
      r.draw(canvas, board);
    }
  }
  else
  {
    if (!symbols.empty())
    {
      node = symbols[symbolIndex].get();
      r.draw(canvas, node);
    }
  }
}

void Scene::preprocessMask(PaintNode* node)
{
  if (maskDirty)
  {
    Scene::ObjectTable = node->PreprocessMask();
    // generate each mask for masked node
    maskDirty = false;
  }
}

void Scene::setPage(int num)
{
  if (num >= 0 && num < artboards.size())
  {
    page = num;
    maskDirty = true;
  }
}

void Scene::nextArtboard()
{
  page = (page + 1 >= artboards.size()) ? page : page + 1;
  maskDirty = true;
}

void Scene::preArtboard()
{
  page = (page - 1 > 0) ? page - 1 : 0;
  maskDirty = true;
}

void Scene::nextSymbol()
{
  symbolIndex = (symbolIndex + 1 >= symbols.size()) ? symbolIndex : symbolIndex + 1;
}

void Scene::prevSymbol()
{
  symbolIndex = (symbolIndex - 1 > 0) ? symbolIndex - 1 : 0;
}

} // namespace VGG
