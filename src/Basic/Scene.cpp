#include "Basic/Scene.hpp"
#include "Basic/Loader.hpp"
#include "Basic/PaintNode.h"
#include "Basic/Renderer.hpp"

#include "include/core/SkCanvas.h"
#include <fstream>
#include <string>
namespace VGG
{

ResourceRepo Scene::ResRepo = ResourceRepo();
ObjectTableType Scene::ObjectTable = {};

void Scene::LoadFileContent(const std::string& json)
{
  LoadFileContent(nlohmann::json::parse(json));
}

void Scene::LoadFileContent(const nlohmann::json& json)
{
  artboards = NlohmannBuilder::fromArtboard(json);
  symbols = NlohmannBuilder::fromSymbolMasters(json);
  page = 0;
  symbolIndex = 0;
  maskDirty = true;
}

void Scene::Render(SkCanvas* canvas)
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

void Scene::SetPage(int num)
{
  if (num >= 0 && num < artboards.size())
  {
    page = num;
    maskDirty = true;
  }
}

void Scene::NextArtboard()
{
  page = (page + 1 >= artboards.size()) ? page : page + 1;
  maskDirty = true;
}

void Scene::PreArtboard()
{
  page = (page - 1 > 0) ? page - 1 : 0;
  maskDirty = true;
}

void Scene::NextSymbol()
{
  symbolIndex = (symbolIndex + 1 >= symbols.size()) ? symbolIndex : symbolIndex + 1;
}

void Scene::PrevSymbol()
{
  symbolIndex = (symbolIndex - 1 > 0) ? symbolIndex - 1 : 0;
}

} // namespace VGG
