#include "Basic/Scene.hpp"
#include "Basic/Loader.hpp"
#include "Basic/Renderer.hpp"

#include "include/core/SkCanvas.h"
#include <fstream>
#include <string>
namespace VGG
{

ResourceRepo Scene::ResRepo = ResourceRepo();

void Scene::LoadFileContent(const std::string& json)
{
  LoadFileContent(nlohmann::json::parse(json));
}

void Scene::LoadFileContent(const nlohmann::json& json)
{
  artboards = fromArtboard(json);
  symbols = fromSymbolMasters(json);
  page = 0;
  symbolIndex = 0;
}

void Scene::Render(SkCanvas* canvas)
{
  PaintNode* node = nullptr;
  SkiaRenderer r;
  if (!renderSymbol)
  {
    auto board = artboards[page].get();
    auto s = board->bound.size();
    r.Draw(canvas, board);
  }
  else
  {
    node = symbols[symbolIndex].get();
    r.Draw(canvas, node);
  }
}

void Scene::NextArtboard()
{
  page = (page + 1 >= artboards.size()) ? page : page + 1;
}

void Scene::PreArtboard()
{
  page = (page - 1 > 0) ? page - 1 : 0;
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
