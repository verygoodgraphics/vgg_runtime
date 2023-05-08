#pragma once
#include <memory>
#include "Basic/Node.hpp"
#include "Basic/PaintNode.h"
#include "include/core/SkCanvas.h"
// #include "vgg_sketch_parser/src/analyze_sketch_file/analyze_sketch_file.h"
#include "nlohmann/json.hpp"
#include "Basic/RenderTreeDef.hpp"

namespace VGG
{
class SymbolMasterNode;

class ArtboardNode;

using ResourceRepo = std::map<std::string, std::vector<char>>;
struct Scene
{
  static ResourceRepo ResRepo;

public:
  std::vector<std::shared_ptr<ArtboardNode>> artboards;
  std::vector<std::shared_ptr<SymbolMasterNode>> symbols;
  int page{ 0 };
  int symbolIndex{ 0 };
  bool renderSymbol{ false };
  bool maskDirty{ true };
  Scene() = default;
  void LoadFileContent(const std::string& json);
  void LoadFileContent(const nlohmann::json& j);
  void Render(SkCanvas* canvas);
  void NextArtboard();
  void PreArtboard();
  void NextSymbol();
  void PrevSymbol();

  static ResourceRepo& getResRepo()
  {
    return ResRepo;
  }

  static void setResRepo(std::map<std::string, std::vector<char>> repo)
  {
    Scene::ResRepo = std::move(repo);
  }

private:
  void preprocessMask(PaintNode* node);
};

}; // namespace VGG
